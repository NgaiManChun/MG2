#include "vertexDivision.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void VertexDivision::Uninit()
	{
		SAFE_RELEASE(s_DataBuffer);
		s_Bookmarks.clear();
		s_EmptyIds.clear();
		s_DataCapcity = 0;
		s_DataCount = 0;
	}

	VertexDivision VertexDivision::Create(unsigned int count, const VERTEX* data) {
		VertexDivision key{};
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

		// 頂点バッファ確保
		if (s_DataCount + count > s_DataCapcity) {
			unsigned int newCapcity = s_DataCount + count * 2;
			if (Buffer::NewVertexBufferCopy(sizeof(VERTEX), newCapcity, s_DataBuffer)) {
				s_DataCapcity = newCapcity;
			}
		}

		// データ転送
		if (data) {
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(VERTEX) * bookmark.offset, sizeof(VERTEX) * (bookmark.offset + bookmark.count));
			Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
		}

		s_DataCount += count;

		return key;
	}

	void VertexDivision::Pad()
	{
		s_DataCount = Buffer::DivisionPadByCS(sizeof(VERTEX), s_DataCount,
			s_Bookmarks,
			s_DataBuffer
		);
	}

	void VertexDivision::SetData(VERTEX* data) {
		BOOKMARK& meta = s_Bookmarks[m_Id];
		D3D11_BOX box = Renderer::GetRangeBox(sizeof(VERTEX) * meta.offset, sizeof(VERTEX) * (meta.offset + meta.count));
		Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
	}
}

