#include "matrixDivision.h"
#include "renderer.h"
#include "MGUtility.h"
#include "matrix4x4.h"

namespace MG {

	void MatrixDivision::Pad()
	{
		// 結果用バッファを作成
		ID3D11Buffer* resultBuffer = Renderer::CreateStructuredBuffer(
			sizeof(Matrix4x4), s_DataSize,
			nullptr,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS
		);
		ID3D11UnorderedAccessView* resultUAV = nullptr;
		if (resultBuffer) {
			resultUAV = Renderer::CreateStructuredUAV(resultBuffer, s_DataSize);
		}
		if (!resultUAV) {
			SAFE_RELEASE(resultBuffer);
			return;
		}

		// ソートしたブックマーク配列を作成
		// ※ID順 != offset順
		std::vector<BOOKMARK*> sortedBookmarks(s_Bookmarks.size());
		for (unsigned int i = 0; i < s_Bookmarks.size(); i++) {
			sortedBookmarks[i] = &s_Bookmarks[i];
		}
		std::sort(sortedBookmarks.begin(), sortedBookmarks.end(),
			[](BOOKMARK* a, BOOKMARK* b) {
				return a->offset < b->offset;
			}
		);

		// データの移動数を計算
		unsigned int last = 0;
		unsigned int totalPad = 0;
		for (unsigned int i = 0; i < sortedBookmarks.size(); i++) {
			if (sortedBookmarks[i]->offset > last) {
				unsigned int start = last;
				unsigned int count = sortedBookmarks[i]->offset - last;
				totalPad += count;
				for (unsigned int j = i; j < sortedBookmarks.size(); j++) {
					sortedBookmarks[j]->padding += count;
				}
			}
			last = sortedBookmarks[i]->offset + sortedBookmarks[i]->count;
		}

		// 移動なしの場合即終了
		if (totalPad == 0) {
			SAFE_RELEASE(resultUAV);
			SAFE_RELEASE(resultBuffer);
			return;
		}

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();
		D3D11_BOX box = Renderer::GetRangeBox(0, sizeof(BOOKMARK) * s_Bookmarks.size());

		// データの移動数をブックマークに更新
		deviceContext->UpdateSubresource(s_BookmarkBuffer, 0, &box, s_Bookmarks.data(), 0, 0);

		// データの移動（コンピュートシェーダ）
		deviceContext->CSSetUnorderedAccessViews(0, 1, &resultUAV, nullptr);
		deviceContext->CSSetShaderResources(0, 1, &s_BookmarkSRV);
		deviceContext->CSSetShaderResources(1, 1, &s_DataSRV);
		CS_CONSTANT constant{ s_Bookmarks.size() };
		Renderer::SetCSContant(constant);
		deviceContext->CSSetShader(s_PadCS, nullptr, 0);
		deviceContext->Dispatch(static_cast<UINT>(ceil((float)s_Bookmarks.size() / 64)), 1, 1);

		// UAV解除
		ID3D11UnorderedAccessView* nullUAV[] = { nullptr , nullptr};
		deviceContext->CSSetUnorderedAccessViews(0, 2, nullUAV, nullptr);

		// 結果を元バッファにコピー
		Renderer::GetDeviceContext()->CopySubresourceRegion(s_DataBuffer, 0, 0, 0, 0, resultBuffer, 0, nullptr);

		// 移動した分のデータ数を減らす
		s_DataSize -= totalPad;

		// 移動後のブックマークデータを更新
		for (unsigned int i = 0; i < s_Bookmarks.size(); i++) {
			s_Bookmarks[i].offset -= s_Bookmarks[i].padding;
			s_Bookmarks[i].padding = 0;
		}
		deviceContext->UpdateSubresource(s_BookmarkBuffer, 0, &box, s_Bookmarks.data(), 0, 0);

		SAFE_RELEASE(resultUAV);
		SAFE_RELEASE(resultBuffer);

	}

	void MatrixDivision::Init()
	{
		if (!s_PadCS) {
			s_PadCS = Renderer::LoadComputeShader("complied_shader\\padMatrixCS.cso");
		}
	}

	void MatrixDivision::Uninit()
	{
		SAFE_RELEASE(s_BookmarkSRV);
		SAFE_RELEASE(s_BookmarkBuffer);
		SAFE_RELEASE(s_DataUAV);
		SAFE_RELEASE(s_DataSRV);
		SAFE_RELEASE(s_DataBuffer);
		SAFE_RELEASE(s_PadCS);
		s_Bookmarks.clear();
		s_EmptyIds.clear();
		s_BookmarkCapcity = 0;
		s_DataCapcity = 0;
		s_DataSize = 0;
	}

	MatrixDivision MatrixDivision::Create(unsigned int count, const Matrix4x4* data) {
		MatrixDivision key{};
		BOOKMARK bookmark{};
		bookmark.offset = s_DataSize;
		bookmark.count = count;

		if (s_EmptyIds.empty()) {
			s_Bookmarks.push_back(bookmark);
			key.m_Id = s_Bookmarks.size() - 1;
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_Bookmarks[key.m_Id] = bookmark;
		}

		// ブックマークバッファ確保
		if (s_Bookmarks.capacity() > s_BookmarkCapcity) {
			unsigned int newCapcity = s_Bookmarks.capacity();
			ID3D11Buffer* newBuffer = Renderer::CreateStructuredBuffer(sizeof(BOOKMARK), newCapcity);
			ID3D11ShaderResourceView* newSrv = nullptr;
			if (newBuffer) {
				newSrv = Renderer::CreateStructuredSRV(newBuffer, newCapcity);
			}
			
			if (newSrv) {
				if (s_BookmarkBuffer) {
					Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, s_BookmarkBuffer, 0, nullptr);
				}
				SAFE_RELEASE(s_BookmarkSRV);
				SAFE_RELEASE(s_BookmarkBuffer);
				s_BookmarkBuffer = newBuffer;
				s_BookmarkSRV = newSrv;
				s_BookmarkCapcity = newCapcity;
			}
			else {
				SAFE_RELEASE(newBuffer);
			}
		}

		// データバッファ確保
		if (s_DataSize + count > s_DataCapcity) {
			unsigned int newCapcity = max(s_DataCapcity + count * 2, DATA_INTERVAL);
			ID3D11Buffer* newBuffer = Renderer::CreateStructuredBuffer(sizeof(Matrix4x4), newCapcity, nullptr, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
			ID3D11ShaderResourceView* newSRV = nullptr;
			ID3D11UnorderedAccessView* newUAV = nullptr;

			if (newBuffer) {
				newSRV = Renderer::CreateStructuredSRV(newBuffer, newCapcity);
				newUAV = Renderer::CreateStructuredUAV(newBuffer, newCapcity);
			}

			if (newSRV && newUAV) {
				if (s_DataBuffer) {
					Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, s_DataBuffer, 0, nullptr);
				}
				SAFE_RELEASE(s_DataUAV);
				SAFE_RELEASE(s_DataSRV);
				SAFE_RELEASE(s_DataBuffer);
				s_DataBuffer = newBuffer;
				s_DataSRV = newSRV;
				s_DataUAV = newUAV;
				s_DataCapcity = newCapcity;
			}
			else {
				SAFE_RELEASE(newUAV);
				SAFE_RELEASE(newSRV);
				SAFE_RELEASE(newBuffer);
			}
		}

		// ブックマークデータ転送
		{
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(BOOKMARK) * key.m_Id, sizeof(BOOKMARK) * (key.m_Id + 1));
			Renderer::GetDeviceContext()->UpdateSubresource(s_BookmarkBuffer, 0, &box, s_Bookmarks.data() + key.m_Id, 0, 0);
		}

		// マトリックスデータ転送
		if (data) {
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(Matrix4x4) * bookmark.offset, sizeof(Matrix4x4) * (bookmark.offset + bookmark.count));
			Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
		}

		s_DataSize += count;

		return key;
	}

	void MatrixDivision::SetData(Matrix4x4* data) {
		BOOKMARK& bookmark = s_Bookmarks[m_Id];
		D3D11_BOX box = Renderer::GetRangeBox(sizeof(Matrix4x4) * bookmark.offset, sizeof(Matrix4x4) * (bookmark.offset + bookmark.count));
		Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
	}
}

