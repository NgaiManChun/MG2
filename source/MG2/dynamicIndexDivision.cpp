#include "dynamicIndexDivision.h"
#include "renderer.h"
#include "MGUtility.h"
#include "buffer.h"

namespace MG {

	void DynamicIndexDivision::Uninit()
	{
		SAFE_RELEASE(s_DataSRV);
		SAFE_RELEASE(s_DataBuffer);
		SAFE_RELEASE(s_BookmarkSRV);
		SAFE_RELEASE(s_BookmarkBuffer);
		s_Data.clear();
		s_Bookmarks.clear();
		s_EmptyIds.clear();
		s_BookmarkCapcity = 0;
		s_DataCapcity = 0;
	}

	void DynamicIndexDivision::Update()
	{
		if (s_NeedUpdateBuffer) {

			// ブックマークバッファ確保
			unsigned int newBookmarkCapcity = static_cast<unsigned int>(s_Bookmarks.capacity());
			if (newBookmarkCapcity > s_BookmarkCapcity) {
				SAFE_RELEASE(s_BookmarkSRV);
				SAFE_RELEASE(s_BookmarkBuffer);
				s_BookmarkBuffer = Renderer::CreateStructuredBuffer(sizeof(BOOKMARK), newBookmarkCapcity, nullptr);
				if (s_BookmarkBuffer) {
					s_BookmarkSRV = Renderer::CreateStructuredSRV(s_BookmarkBuffer, newBookmarkCapcity);
					s_BookmarkCapcity = newBookmarkCapcity;
				}
			}

			// データバッファ確保
			unsigned int newDataCapcity = static_cast<unsigned int>(s_Data.capacity());
			if (newDataCapcity > s_DataCapcity) {
				SAFE_RELEASE(s_DataSRV);
				SAFE_RELEASE(s_DataBuffer);
				s_DataBuffer = Renderer::CreateStructuredBuffer(sizeof(unsigned int), newDataCapcity, nullptr);
				if (s_DataBuffer) {
					s_DataSRV = Renderer::CreateStructuredSRV(s_DataBuffer, newDataCapcity);
					s_DataCapcity = newDataCapcity;
				}
			}

			// データ転送
			if (s_BookmarkBuffer && s_BookmarkSRV && s_DataBuffer && s_DataSRV) {

				D3D11_BOX box = Renderer::GetRangeBox(0, static_cast<unsigned int>(sizeof(BOOKMARK) * s_Bookmarks.size()));
				Renderer::GetDeviceContext()->UpdateSubresource(s_BookmarkBuffer, 0, &box, s_Bookmarks.data(), 0, 0);

				box = Renderer::GetRangeBox(0, static_cast<unsigned int>(sizeof(unsigned int) * s_Data.size()));
				Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, s_Data.data(), 0, 0);

				s_NeedUpdateBuffer = false;
			}
		}
	}

	DynamicIndexDivision DynamicIndexDivision::Create(unsigned int count, unsigned int* data)
	{
		DynamicIndexDivision key{};
		BOOKMARK bookmark{};
		bookmark.offset = static_cast<unsigned int>(s_Data.size());
		bookmark.count = count;

		if (s_EmptyIds.empty()) {
			s_Bookmarks.push_back(bookmark);
			key.m_Id = static_cast<unsigned int>(s_Bookmarks.size() - 1);
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_Bookmarks[key.m_Id] = bookmark;
		}

		if (data) {
			for (unsigned int i = 0; i < count; i++) {
				s_Data.push_back(data[i]);
			}
		}
		else {
			for (unsigned int i = 0; i < count; i++) {
				s_Data.push_back(UINT_MAX);
			}
		}

		s_NeedUpdateBuffer = true;

		return key;
	}

	void DynamicIndexDivision::Pad()
	{

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
		if (totalPad == 0) return;

		// データの移動
		std::vector<unsigned int> copyData = s_Data;
		for (unsigned int i = 0; i < s_Bookmarks.size(); i++) {
			if (s_Bookmarks[i].padding > 0) {
				memcpy(s_Data.data() + s_Bookmarks[i].offset - s_Bookmarks[i].padding, copyData.data() + s_Bookmarks[i].offset, sizeof(unsigned int) * s_Bookmarks[i].count);
				s_Bookmarks[i].offset -= s_Bookmarks[i].padding;
				s_Bookmarks[i].padding = 0;
			}
		}
		s_Data.resize(s_Data.size() - totalPad);

	}

} // namespace MG

