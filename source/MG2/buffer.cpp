#include "buffer.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG::Buffer {

	bool NewVertexBufferCopy(unsigned int stride, unsigned int capcity, ID3D11Buffer*& buffer) {
		ID3D11Buffer* newBuffer = Renderer::CreateVertexBuffer(stride * capcity);
		if (!newBuffer) return false;

		if (buffer) {
			Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, buffer, 0, nullptr);
		}
		SAFE_RELEASE(buffer);
		buffer = newBuffer;
		return true;
	}

	bool NewBufferCopy(unsigned int stride, unsigned int capcity, ID3D11Buffer*& buffer, ID3D11ShaderResourceView*& srv) {
		ID3D11Buffer* newBuffer = Renderer::CreateStructuredBuffer(stride, capcity);
		ID3D11ShaderResourceView* newSRV = nullptr;
		if (newBuffer) {
			newSRV = Renderer::CreateStructuredSRV(newBuffer, capcity);
		}

		if (!newSRV) {
			SAFE_RELEASE(newBuffer);
			return false;
		}

		if (buffer) {
			Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, buffer, 0, nullptr);
		}
		SAFE_RELEASE(srv);
		SAFE_RELEASE(buffer);
		srv = newSRV;
		buffer = newBuffer;
		return true;
	}

	bool NewBufferCopy(unsigned int stride, unsigned int capcity, ID3D11Buffer*& buffer, ID3D11ShaderResourceView*& srv, ID3D11UnorderedAccessView*& uav) {
		ID3D11Buffer* newBuffer = Renderer::CreateStructuredBuffer(stride, capcity, nullptr, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
		ID3D11ShaderResourceView* newSRV = nullptr;
		ID3D11UnorderedAccessView* newUAV = nullptr;
		if (newBuffer) {
			newSRV = Renderer::CreateStructuredSRV(newBuffer, capcity);
			newUAV = Renderer::CreateStructuredUAV(newBuffer, capcity);
		}

		if (!newSRV || !newUAV) {
			SAFE_RELEASE(newSRV);
			SAFE_RELEASE(newUAV);
			SAFE_RELEASE(newBuffer);
			return false;
		}

		if (buffer) {
			Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, buffer, 0, nullptr);
		}
		SAFE_RELEASE(uav);
		SAFE_RELEASE(srv);
		SAFE_RELEASE(buffer);
		uav = newUAV;
		srv = newSRV;
		buffer = newBuffer;
		return true;
	}

	unsigned int DivisionPadByCS(unsigned int stride, unsigned int dataCount,
		std::vector<BOOKMARK>& bookmarks,
		ID3D11Buffer* dataBuffer,
		ID3D11Buffer* returnBookmarkBuffer
	)
	{
		
		static ID3D11ComputeShader* s_PaddingCS = Renderer::LoadComputeShader("complied_shader\\paddingCS.cso");

		unsigned int bookmarkCount = bookmarks.size();
		if (bookmarkCount > s_BookmarkCapcity) {
			SAFE_RELEASE(s_BookmarkSRV);
			SAFE_RELEASE(s_BookmarkBuffer);
			s_BookmarkBuffer = Renderer::CreateStructuredBuffer(sizeof(BOOKMARK), bookmarkCount);
			s_BookmarkSRV = Renderer::CreateStructuredSRV(s_BookmarkBuffer, bookmarkCount);
			
		}

		unsigned int byteWidth = stride * dataCount;
		if (byteWidth > s_DataCapcity) {
			SAFE_RELEASE(s_InputSRV);
			SAFE_RELEASE(s_ResultUAV);
			SAFE_RELEASE(s_InputBuffer);
			SAFE_RELEASE(s_ResultBuffer);
			s_InputBuffer = Renderer::CreateByteAddressBuffer(byteWidth, nullptr);
			s_ResultBuffer = Renderer::CreateByteAddressBuffer(byteWidth, nullptr, D3D11_BIND_UNORDERED_ACCESS);
			s_InputSRV = Renderer::CreateByteAddressSRV(s_InputBuffer, byteWidth);
			s_ResultUAV = Renderer::CreateByteAddressUAV(s_ResultBuffer, byteWidth);
		}

		if (!s_BookmarkSRV || !s_InputSRV || !s_ResultUAV) {
			return dataCount;
		}

		s_BookmarkCapcity = bookmarkCount;
		s_DataCapcity = byteWidth;

		// ソートしたブックマーク配列を作成
		// ※ID順 != offset順
		std::vector<BOOKMARK*> sortedBookmarks(bookmarkCount);
		for (unsigned int i = 0; i < sortedBookmarks.size(); i++) {
			sortedBookmarks[i] = &bookmarks[i];
		}
		std::sort(sortedBookmarks.begin(), sortedBookmarks.end(),
			[](BOOKMARK* a, BOOKMARK* b) {
				return a->offset < b->offset;
			}
		);

		// 各disvision詰める数を計算
		unsigned int last = 0;
		unsigned int totalPad = 0;
		for (unsigned int i = 0; i < sortedBookmarks.size(); i++) {
			if (sortedBookmarks[i]->offset > last) {
				unsigned int count = sortedBookmarks[i]->offset - last;
				totalPad += count;
				for (unsigned int j = i; j < sortedBookmarks.size(); j++) {
					sortedBookmarks[j]->padding += count;
				}
			}
			last = sortedBookmarks[i]->offset + sortedBookmarks[i]->count;
		}

		if (totalPad == 0) {
			return dataCount;
		}

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		D3D11_BOX dataBox = Renderer::GetRangeBox(0, byteWidth);
		deviceContext->CopySubresourceRegion(s_InputBuffer, 0, 0, 0, 0, dataBuffer, 0, &dataBox);

		D3D11_BOX box = Renderer::GetRangeBox(0, sizeof(BOOKMARK) * bookmarkCount);

		// ブックマークを更新
		deviceContext->UpdateSubresource(s_BookmarkBuffer, 0, &box, bookmarks.data(), 0, 0);

		// pad data
		deviceContext->CSSetUnorderedAccessViews(0, 1, &s_ResultUAV, nullptr);
		ID3D11ShaderResourceView* SRVs[] = {
			s_BookmarkSRV,
			s_InputSRV
		};
		deviceContext->CSSetShaderResources(0, 2, SRVs);
		CS_CONSTANT constant{ 
			stride / 4, // 4byteずつコピー 
			bookmarkCount
		};
		Renderer::SetCSContant(constant);
		deviceContext->CSSetShader(s_PaddingCS, nullptr, 0);
		deviceContext->Dispatch(static_cast<UINT>(ceil((float)constant.CSMaxX / 8)), static_cast<UINT>(ceil((float)bookmarkCount / 8)), 1);

		// UAV解除
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
		deviceContext->CopySubresourceRegion(dataBuffer, 0, 0, 0, 0, s_ResultBuffer, 0, &dataBox);

		for (unsigned int i = 0; i < bookmarkCount; i++) {
			bookmarks[i].offset -= bookmarks[i].padding;
			bookmarks[i].padding = 0;
		}
		if (returnBookmarkBuffer) {
			deviceContext->UpdateSubresource(returnBookmarkBuffer, 0, &box, bookmarks.data(), 0, 0);
		}

		return dataCount - totalPad;
	}
	
} // namespace MG::Buffer

