#include "dynamicIndexDivision.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {
	DynamicIndexDivision DynamicIndexDivision::Create(unsigned int count, unsigned int* data)
	{
		DynamicIndexDivision key{};
		META meta{};
		meta.offset = s_Data.size();
		meta.count = count;

		if (s_EmptyIds.empty()) {
			s_Meta.push_back(meta);
			key.m_Id = s_Meta.size() - 1;
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_Meta[key.m_Id] = meta;
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
		std::vector<META*> sortedMeta(s_Meta.size());
		for (unsigned int i = 0; i < s_Meta.size(); i++) {
			sortedMeta[i] = &s_Meta[i];
		}

		std::sort(sortedMeta.begin(), sortedMeta.end(),
			[](META* a, META* b) {
				return a->offset < b->offset;
			}
		);

		unsigned int last = 0;
		unsigned int totalPad = 0;
		for (unsigned int i = 0; i < sortedMeta.size(); i++) {
			if (sortedMeta[i]->offset > last) {
				unsigned int start = last;
				unsigned int count = sortedMeta[i]->offset - last;
				totalPad += count;
				for (unsigned int j = i; j < sortedMeta.size(); j++) {
					sortedMeta[j]->padding += count;
				}
			}
			last = sortedMeta[i]->offset + sortedMeta[i]->count;
		}

		if (totalPad == 0) return;
		std::vector<unsigned int> copyData = s_Data;
		for (unsigned int i = 0; i < s_Meta.size(); i++) {
			if (s_Meta[i].padding > 0) {
				memcpy(s_Data.data() + s_Meta[i].offset - s_Meta[i].padding, copyData.data() + s_Meta[i].offset, sizeof(unsigned int) * s_Meta[i].count);
				s_Meta[i].offset -= s_Meta[i].padding;
				s_Meta[i].padding = 0;
			}
		}

		s_Data.resize(s_Data.size() - totalPad);

	}

	void DynamicIndexDivision::Uninit()
	{
		SAFE_RELEASE(s_DataSRV);
		SAFE_RELEASE(s_DataBuffer);
		SAFE_RELEASE(s_MetaSRV);
		SAFE_RELEASE(s_MetaBuffer);
		s_Data.clear();
		s_Meta.clear();
		s_EmptyIds.clear();
		s_MetaCapcity = 0;
		s_DataCapcity = 0;
	}

	void DynamicIndexDivision::Update()
	{
		if (s_NeedUpdateBuffer) {
			if (s_Meta.capacity() > s_MetaCapcity) {
				SAFE_RELEASE(s_MetaSRV);
				SAFE_RELEASE(s_MetaBuffer);
				s_MetaBuffer = Renderer::CreateStructuredBuffer(sizeof(META), s_Meta.capacity(), nullptr);
				if (s_MetaBuffer) {
					s_MetaSRV = Renderer::CreateStructuredSRV(s_MetaBuffer, s_Meta.capacity());
					s_MetaCapcity = s_Data.capacity();
				}
			}

			if (s_Data.capacity() > s_DataCapcity) {
				SAFE_RELEASE(s_DataSRV);
				SAFE_RELEASE(s_DataBuffer);
				s_DataBuffer = Renderer::CreateStructuredBuffer(sizeof(unsigned int), s_Data.capacity(), nullptr);
				if (s_DataBuffer) {
					s_DataSRV = Renderer::CreateStructuredSRV(s_DataBuffer, s_Data.capacity());
					s_DataCapcity = s_Data.capacity();
				}
			}

			if (s_MetaBuffer && s_MetaSRV && s_DataBuffer && s_DataSRV) {

				D3D11_BOX box = Renderer::GetRangeBox(0, sizeof(META) * s_Meta.size());
				Renderer::GetDeviceContext()->UpdateSubresource(s_MetaBuffer, 0, &box, s_Meta.data(), 0, 0);

				box = Renderer::GetRangeBox(0, sizeof(unsigned int) * s_Data.size());
				Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, s_Data.data(), 0, 0);

				s_NeedUpdateBuffer = false;
			}
		}
	}
} // namespace MG

