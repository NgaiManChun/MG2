#include "vertexIndexDivision.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void VertexIndexDivision::Uninit()
	{
		SAFE_RELEASE(s_DataBuffer);
		s_Bookmarks.clear();
		s_EmptyIds.clear();
	}

	VertexIndexDivision VertexIndexDivision::Create(unsigned int count, const unsigned int* data) {
		VertexIndexDivision key{};
		BOOKMARK meta{};
		meta.offset = s_DataCount;
		meta.count = count;

		if (s_EmptyIds.empty()) {
			s_Bookmarks.push_back(meta);
			key.m_Id = s_Bookmarks.size() - 1;
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_Bookmarks[key.m_Id] = meta;
		}

		// 頂点バッファ確保
		if (s_DataCount + count > s_DataCapcity) {
			unsigned int newCapcity = s_DataCount + count * 2;
			if (Buffer::NewVertexBufferCopy(sizeof(unsigned int), newCapcity, s_DataBuffer)) {
				s_DataCapcity = newCapcity;
			}
		}

		// データ転送
		if (data) {
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(unsigned int) * meta.offset, sizeof(unsigned int) * (meta.offset + meta.count));
			Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
		}
		s_DataCount += count;

		return key;
	}

	void VertexIndexDivision::Pad()
	{
		s_DataCount = Buffer::DivisionPadByCS(sizeof(unsigned int), s_DataCount,
			s_Bookmarks,
			s_DataBuffer
		);
	}

	void VertexIndexDivision::SetData(const unsigned int* data) {
		BOOKMARK& meta = s_Bookmarks[m_Id];
		D3D11_BOX box = Renderer::GetRangeBox(sizeof(unsigned int) * meta.offset, sizeof(unsigned int) * (meta.offset + meta.count));
		Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
	}
}

