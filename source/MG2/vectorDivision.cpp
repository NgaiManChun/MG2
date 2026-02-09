#include "vectorDivision.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	bool VectorDivision::ReserveMeta(unsigned int newCapcity)
	{
		if (newCapcity < s_Meta.size()) return false;

		ID3D11Buffer* newBuffer = Renderer::CreateStructuredBuffer(sizeof(BOOKMARK), newCapcity);
		if (!newBuffer) return false;

		ID3D11ShaderResourceView* newSrv = Renderer::CreateStructuredSRV(newBuffer, newCapcity);
		if (!newSrv) {
			SAFE_RELEASE(newBuffer);
			return false;
		}

		if (s_MetaBuffer) {
			Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, s_MetaBuffer, 0, nullptr);
		}

		SAFE_RELEASE(s_MetaSRV);
		SAFE_RELEASE(s_MetaBuffer);
		s_MetaBuffer = newBuffer;
		s_MetaSRV = newSrv;
		s_MetaCapcity = newCapcity;
		return true;
	}
	bool VectorDivision::ReserveData(unsigned int newCapcity)
	{
		if (newCapcity < s_DataSize) return false;

		ID3D11Buffer* newBuffer = Renderer::CreateStructuredBuffer(sizeof(Vector4), newCapcity, nullptr, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
		if (!newBuffer) return false;

		ID3D11ShaderResourceView* newSRV = Renderer::CreateStructuredSRV(newBuffer, newCapcity);
		ID3D11UnorderedAccessView* newUAV = Renderer::CreateStructuredUAV(newBuffer, newCapcity);
		if (!newSRV || !newUAV) {
			SAFE_RELEASE(newUAV);
			SAFE_RELEASE(newSRV);
			SAFE_RELEASE(newBuffer);
			return false;
		}

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
		return true;
	}

	void VectorDivision::Pad()
	{

		ID3D11Buffer* resultBuffer = Renderer::CreateStructuredBuffer(
			sizeof(Vector4), s_DataSize,
			nullptr,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS
		);
		if (!resultBuffer) return;
		ID3D11UnorderedAccessView* resultUAV = Renderer::CreateStructuredUAV(resultBuffer, s_DataSize);
		if (!resultUAV) {
			SAFE_RELEASE(resultBuffer);
			return;
		}

		std::vector<BOOKMARK*> sortedMeta(s_Meta.size());
		for (unsigned int i = 0; i < s_Meta.size(); i++) {
			sortedMeta[i] = &s_Meta[i];
		}

		std::sort(sortedMeta.begin(), sortedMeta.end(),
			[](BOOKMARK* a, BOOKMARK* b) {
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

		if (totalPad == 0) {
			SAFE_RELEASE(resultUAV);
			SAFE_RELEASE(resultBuffer);
			return;
		}

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();
		D3D11_BOX box = Renderer::GetRangeBox(0, static_cast<unsigned int>(sizeof(BOOKMARK) * s_Meta.size()));

		// update meta
		deviceContext->UpdateSubresource(s_MetaBuffer, 0, &box, s_Meta.data(), 0, 0);

		// pad data
		deviceContext->CSSetUnorderedAccessViews(0, 1, &resultUAV, nullptr);
		deviceContext->CSSetShaderResources(0, 1, &s_MetaSRV);
		deviceContext->CSSetShaderResources(1, 1, &s_DataSRV);
		CS_CONSTANT constant{ static_cast<unsigned int>(s_Meta.size()) };
		Renderer::SetCSContant(constant);
		deviceContext->CSSetShader(s_PadCS, nullptr, 0);
		deviceContext->Dispatch(static_cast<UINT>(ceil((float)s_Meta.size() / 64)), 1, 1);
		// UAV‰ðœ
		ID3D11UnorderedAccessView* nullUAV[] = { nullptr , nullptr };
		deviceContext->CSSetUnorderedAccessViews(0, 2, nullUAV, nullptr);
		Renderer::GetDeviceContext()->CopySubresourceRegion(s_DataBuffer, 0, 0, 0, 0, resultBuffer, 0, nullptr);

		s_DataSize -= totalPad;

		for (unsigned int i = 0; i < s_Meta.size(); i++) {
			s_Meta[i].offset -= s_Meta[i].padding;
			s_Meta[i].padding = 0;
		}

		// update meta
		deviceContext->UpdateSubresource(s_MetaBuffer, 0, &box, s_Meta.data(), 0, 0);

		SAFE_RELEASE(resultUAV);
		SAFE_RELEASE(resultBuffer);

	}

	void VectorDivision::Init()
	{
		if (!s_PadCS) {
			s_PadCS = Renderer::LoadComputeShader("complied_shader\\padMatrixCS.cso");
		}
	}

	void VectorDivision::Uninit()
	{
		SAFE_RELEASE(s_MetaSRV);
		SAFE_RELEASE(s_MetaBuffer);
		SAFE_RELEASE(s_DataUAV);
		SAFE_RELEASE(s_DataSRV);
		SAFE_RELEASE(s_DataBuffer);
		SAFE_RELEASE(s_PadCS);
		s_Meta.clear();
		s_EmptyIds.clear();
	}

	VectorDivision VectorDivision::Create(unsigned int count, const Vector4* data) {
		VectorDivision key{};
		BOOKMARK meta{};
		meta.offset = s_DataSize;
		meta.count = count;

		if (s_Meta.size() + 1 > s_MetaCapcity) {
			unsigned int newCapcity = s_MetaCapcity + META_INTERVAL;
			if (!ReserveMeta(newCapcity)) {
				return key;
			}
		}

		if (s_DataSize + count > s_DataCapcity) {
			unsigned int newCapcity = max(s_DataCapcity + count * 2, DATA_INTERVAL);
			if (!ReserveData(newCapcity)) {
				return key;
			}
		}

		if (s_EmptyIds.empty()) {
			s_Meta.push_back(meta);
			key.m_Id = static_cast<unsigned int>(s_Meta.size() - 1);
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_Meta[key.m_Id] = meta;
		}

		// update meta
		{
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(BOOKMARK) * key.m_Id, sizeof(BOOKMARK) * (key.m_Id + 1));
			Renderer::GetDeviceContext()->UpdateSubresource(s_MetaBuffer, 0, &box, s_Meta.data() + key.m_Id, 0, 0);
		}

		// update data
		if (data) {
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(Vector4) * meta.offset, sizeof(Vector4) * (meta.offset + meta.count));
			Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
		}
		s_DataSize += count;

		return key;
	}

	void VectorDivision::SetData(Vector4* data) {
		BOOKMARK& meta = s_Meta[m_Id];
		D3D11_BOX box = Renderer::GetRangeBox(sizeof(Vector4) * meta.offset, sizeof(Vector4) * (meta.offset + meta.count));
		Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
	}
}

