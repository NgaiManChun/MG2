#include "transformDivision.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	bool TransformDivision::ReserveMeta(unsigned int newCapcity)
	{
		if (newCapcity < s_Bookmarks.size()) return false;

		ID3D11Buffer* newBuffer = Renderer::CreateStructuredBuffer(sizeof(BOOKMARK), newCapcity);
		if (!newBuffer) return false;

		ID3D11ShaderResourceView* newSrv = Renderer::CreateStructuredSRV(newBuffer, newCapcity);
		if (!newSrv) {
			SAFE_RELEASE(s_BookmarkBuffer);
			return false;
		} 

		if (s_BookmarkBuffer) {
			Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, s_BookmarkBuffer, 0, nullptr);
		}

		SAFE_RELEASE(s_BookmarkSRV);
		SAFE_RELEASE(s_BookmarkBuffer);
		s_BookmarkBuffer = newBuffer;
		s_BookmarkSRV = newSrv;
		s_BookmarkCapcity = newCapcity;
		return true;
	}
	bool TransformDivision::ReserveData(unsigned int newCapcity)
	{
		if (newCapcity < s_DataSize) return false;

		ID3D11Buffer* newBuffer = Renderer::CreateStructuredBuffer(sizeof(TRANSFORM), newCapcity);
		if (!newBuffer) return false;

		ID3D11ShaderResourceView* newSrv = Renderer::CreateStructuredSRV(newBuffer, newCapcity);
		if (!newSrv) {
			SAFE_RELEASE(s_DataBuffer);
			return false;
		}

		if (s_DataBuffer) {
			Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, s_DataBuffer, 0, nullptr);
		}

		SAFE_RELEASE(s_DataSRV);
		SAFE_RELEASE(s_DataBuffer);
		s_DataBuffer = newBuffer;
		s_DataSRV = newSrv;
		s_DataCapcity = newCapcity;
		return true;
	}

	void TransformDivision::Pad()
	{
		// 結果用バッファを作成
		ID3D11Buffer* resultBuffer = Renderer::CreateStructuredBuffer(
			sizeof(TRANSFORM), s_DataSize, 
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
		ID3D11UnorderedAccessView* nullUAV[] = { nullptr , nullptr };
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

	void TransformDivision::Init()
	{ 
		if (!s_PadCS) {
			s_PadCS = Renderer::LoadComputeShader("complied_shader\\padTransformCS.cso");
		}
	}

	void TransformDivision::Uninit()
	{
		SAFE_RELEASE(s_BookmarkSRV);
		SAFE_RELEASE(s_BookmarkBuffer);
		SAFE_RELEASE(s_DataSRV);
		SAFE_RELEASE(s_DataUAV);
		SAFE_RELEASE(s_DataBuffer);
		SAFE_RELEASE(s_PadCS);
		s_Bookmarks.clear();
		s_EmptyIds.clear();
		s_BookmarkCapcity = 0;
		s_DataCapcity = 0;
		s_DataSize = 0;
	}

	TransformDivision TransformDivision::Create(unsigned int count, const TRANSFORM* data) {
		TransformDivision key{};
		BOOKMARK meta{};
		meta.offset = s_DataSize;
		meta.count = count;

		/*if (s_Bookmarks.size() + 1 > s_BookmarkCapcity) {
			unsigned int newCapcity = s_BookmarkCapcity + META_INTERVAL;
			if (!ReserveMeta(newCapcity)) {
				return key;
			}
		}*/

		/*if (s_DataSize + count > s_DataCapcity) {
			unsigned int newCapcity = max(s_DataCapcity + count * 2, DATA_INTERVAL);
			if (!ReserveData(newCapcity)) {
				return key;
			}
		}*/

		if (s_EmptyIds.empty()) {
			s_Bookmarks.push_back(meta);
			key.m_Id = s_Bookmarks.size() - 1;
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_Bookmarks[key.m_Id] = meta;
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
				Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, s_BookmarkBuffer, 0, nullptr);
				SAFE_RELEASE(s_BookmarkSRV);
				SAFE_RELEASE(s_BookmarkBuffer);
				s_BookmarkSRV = newSrv;
				s_BookmarkBuffer = newBuffer;
				s_BookmarkCapcity = newCapcity;
			}
			else {
				SAFE_RELEASE(newBuffer);
			}
		}

		// データバッファ確保
		if (s_DataSize + count > s_DataCapcity) {
			unsigned int newCapcity = max(s_DataCapcity + count * 2, DATA_INTERVAL);

			ID3D11Buffer* newBuffer = Renderer::CreateStructuredBuffer(sizeof(TRANSFORM), newCapcity, nullptr);
			ID3D11ShaderResourceView* newSRV = nullptr;
			if (newBuffer) {
				newSRV = Renderer::CreateStructuredSRV(newBuffer, newCapcity);
			}
			if (newSRV) {
				if (s_DataBuffer) {
					Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, s_DataBuffer, 0, nullptr);
				}
				SAFE_RELEASE(s_DataSRV);
				SAFE_RELEASE(s_DataBuffer);
				s_DataBuffer = newBuffer;
				s_DataSRV = newSRV;
				s_DataCapcity = newCapcity;
			}
			else {
				SAFE_RELEASE(newBuffer);
			}
		}

		// update meta
		D3D11_BOX box = Renderer::GetRangeBox(sizeof(BOOKMARK) * key.m_Id, sizeof(BOOKMARK) * (key.m_Id + 1));
		Renderer::GetDeviceContext()->UpdateSubresource(s_BookmarkBuffer, 0, &box, s_Bookmarks.data() + key.m_Id, 0, 0);

		// update data
		if (data) {
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(TRANSFORM) * meta.offset, sizeof(TRANSFORM) * (meta.offset + meta.count));
			Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
		}
		s_DataSize += count;

		return key;
	}

	void TransformDivision::SetData(TRANSFORM* data) {
		BOOKMARK& bookmark = s_Bookmarks[m_Id];
		D3D11_BOX box = Renderer::GetRangeBox(sizeof(TRANSFORM) * bookmark.offset, sizeof(TRANSFORM) * (bookmark.offset + bookmark.count));
		Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
	}
}

