#include "matrixDivision.h"
#include "renderer.h"
#include "MGUtility.h"
#include "buffer.h"

namespace MG {

	void MatrixDivision::Uninit()
	{
		SAFE_RELEASE(s_BookmarkSRV);
		SAFE_RELEASE(s_BookmarkBuffer);
		SAFE_RELEASE(s_DataUAV);
		SAFE_RELEASE(s_DataSRV);
		SAFE_RELEASE(s_DataBuffer);
		s_Bookmarks.clear();
		s_EmptyIds.clear();
		s_BookmarkCapcity = 0;
		s_DataCapcity = 0;
		s_DataCount = 0;
	}

	MatrixDivision MatrixDivision::Create(unsigned int count, const Matrix4x4* data) {
		MatrixDivision key{};
		BOOKMARK bookmark{};
		bookmark.offset = s_DataCount;
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

		// ブックマークバッファ確保
		if (s_Bookmarks.capacity() > s_BookmarkCapcity) {
			unsigned int newCapcity = static_cast<unsigned int>(s_Bookmarks.capacity());
			if (Buffer::NewBufferCopy(sizeof(BOOKMARK), newCapcity, s_BookmarkBuffer, s_BookmarkSRV)) {
				s_BookmarkCapcity = newCapcity;
			}
		}

		// データバッファ確保
		if (s_DataCount + count > s_DataCapcity) {
			unsigned int newCapcity = s_DataCount + count * 2;
			if (Buffer::NewBufferCopy(sizeof(Matrix4x4), newCapcity, s_DataBuffer, s_DataSRV, s_DataUAV)) {
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
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(Matrix4x4) * bookmark.offset, sizeof(Matrix4x4) * (bookmark.offset + bookmark.count));
			Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
		}

		s_DataCount += count;

		return key;
	}

	void MatrixDivision::Pad()
	{
		s_DataCount = Buffer::DivisionPadByCS(sizeof(Matrix4x4), s_DataCount,
			s_Bookmarks,
			s_DataBuffer,
			s_BookmarkBuffer
		);
	}

	void MatrixDivision::SetData(Matrix4x4* data) {
		BOOKMARK& bookmark = s_Bookmarks[m_Id];
		D3D11_BOX box = Renderer::GetRangeBox(sizeof(Matrix4x4) * bookmark.offset, sizeof(Matrix4x4) * (bookmark.offset + bookmark.count));
		Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
	}
}

