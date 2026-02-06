#include "boneDivision.h"
#include "renderer.h"
#include "MGUtility.h"
#include "buffer.h"

namespace MG {

	void BoneDivision::Uninit()
	{
		SAFE_RELEASE(s_BookmarkSRV);
		SAFE_RELEASE(s_BookmarkBuffer);
		SAFE_RELEASE(s_DataSRV);
		SAFE_RELEASE(s_DataBuffer);
		SAFE_RELEASE(s_PaddingCS);
		s_Bookmarks.clear();
		s_EmptyIds.clear();
		s_BookmarkCapcity = 0;
		s_DataCapcity = 0;
		s_DataCount = 0;
	}

	BoneDivision BoneDivision::Create(unsigned int count, const BONE* data) {
		BoneDivision key{};
		BOOKMARK bookmark{};
		bookmark.offset = s_DataCount;
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
			if (Buffer::NewBufferCopy(sizeof(BOOKMARK), newCapcity, s_BookmarkBuffer, s_BookmarkSRV)) {
				s_BookmarkCapcity = newCapcity;
			}
		}

		// 本データバッファ確保
		if (s_DataCount + count > s_DataCapcity) {
			unsigned int newCapcity = s_DataCount + count * 2;
			if (Buffer::NewBufferCopy(sizeof(BONE), newCapcity, s_DataBuffer, s_DataSRV)) {
				s_DataCapcity = newCapcity;
			}
		}

		// ブックマークデータ転送
		{
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(BOOKMARK) * key.m_Id, sizeof(BOOKMARK) * (key.m_Id + 1));
			Renderer::GetDeviceContext()->UpdateSubresource(s_BookmarkBuffer, 0, &box, s_Bookmarks.data() + key.m_Id, 0, 0);
		}

		// 本データ転送
		if (data) {
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(BONE) * bookmark.offset, sizeof(BONE) * (bookmark.offset + bookmark.count));
			Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
		}

		s_DataCount += count;

		return key;
	}

	void BoneDivision::Pad()
	{
		s_DataCount = Buffer::DivisionPadByCS(sizeof(BONE), s_DataCount,
			s_Bookmarks,
			s_DataBuffer,
			s_BookmarkBuffer
		);
	}
}

