#include "vertexDivision.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	bool VertexDivision::ReserveData(unsigned int newCapcity)
	{
		if (newCapcity < s_DataCount) return false;

		ID3D11Buffer* newBuffer = Renderer::CreateVertexBuffer(sizeof(VERTEX) * newCapcity);
		if (!newBuffer) return false;

		if (s_DataBuffer) {
			Renderer::GetDeviceContext()->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, s_DataBuffer, 0, nullptr);
		}

		SAFE_RELEASE(s_DataBuffer);
		s_DataBuffer = newBuffer;
		s_DataCapcity = newCapcity;
		return true;
	}

	void VertexDivision::Pad()
	{
		ID3D11Buffer* copyBuffer = Renderer::CreateStructuredBuffer(sizeof(VERTEX), s_DataCount);
		ID3D11Buffer* metaBuffer = Renderer::CreateStructuredBuffer(sizeof(META), s_Meta.size());
		ID3D11Buffer* resultBuffer = Renderer::CreateStructuredBuffer(
			sizeof(VERTEX), s_DataCount,
			nullptr,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS
		);

		if (!copyBuffer || !metaBuffer || !resultBuffer) {
			SAFE_RELEASE(copyBuffer);
			SAFE_RELEASE(metaBuffer);
			SAFE_RELEASE(resultBuffer);
			return;
		}

		ID3D11ShaderResourceView* copySRV = Renderer::CreateStructuredSRV(copyBuffer, s_DataCount);
		ID3D11ShaderResourceView* metaSRV = Renderer::CreateStructuredSRV(metaBuffer, s_Meta.size());
		ID3D11UnorderedAccessView* resultUAV = Renderer::CreateStructuredUAV(resultBuffer, s_DataCount);
		if (!copySRV || !metaSRV || !resultUAV) {
			SAFE_RELEASE(copySRV);
			SAFE_RELEASE(metaSRV);
			SAFE_RELEASE(resultUAV);
			SAFE_RELEASE(copyBuffer);
			SAFE_RELEASE(metaBuffer);
			SAFE_RELEASE(resultBuffer);
			return;
		}

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		// 一旦StructuredBufferにコピー
		deviceContext->CopySubresourceRegion(copyBuffer, 0, 0, 0, 0, s_DataBuffer, 0, nullptr);

		// 本来のデータをそのままにポインタをoffset順にソート
		std::vector<META*> sortedMeta(s_Meta.size());
		for (unsigned int i = 0; i < s_Meta.size(); i++) {
			sortedMeta[i] = &s_Meta[i];
		}
		std::sort(sortedMeta.begin(), sortedMeta.end(),
			[](META* a, META* b) {
				return a->offset < b->offset;
			}
		);

		// 各disvision詰める数を計算
		unsigned int last = 0;
		unsigned int totalPad = 0;
		for (unsigned int i = 0; i < sortedMeta.size(); i++) {
			if (sortedMeta[i]->offset > last) {
				unsigned int count = sortedMeta[i]->offset - last;
				totalPad += count;
				for (unsigned int j = i; j < sortedMeta.size(); j++) {
					sortedMeta[j]->padding += count;
				}
			}
			last = sortedMeta[i]->offset + sortedMeta[i]->count;
		}

		if (totalPad == 0) {
			SAFE_RELEASE(copySRV);
			SAFE_RELEASE(metaSRV);
			SAFE_RELEASE(resultUAV);
			SAFE_RELEASE(copyBuffer);
			SAFE_RELEASE(metaBuffer);
			SAFE_RELEASE(resultBuffer);
			return;
		}

		
		D3D11_BOX box = Renderer::GetRangeBox(0, sizeof(META) * s_Meta.size());

		// update meta
		deviceContext->UpdateSubresource(metaBuffer, 0, &box, s_Meta.data(), 0, 0);

		// pad data
		deviceContext->CSSetUnorderedAccessViews(0, 1, &resultUAV, nullptr);
		ID3D11ShaderResourceView* SRVs[] = {
			metaSRV,
			copySRV
		};
		deviceContext->CSSetShaderResources(0, 2, SRVs);
		CS_CONSTANT constant{ s_Meta.size() };
		Renderer::SetCSContant(constant);
		deviceContext->CSSetShader(s_PadCS, nullptr, 0);
		deviceContext->Dispatch(static_cast<UINT>(ceil((float)s_Meta.size() / 64)), 1, 1);

		// UAV解除
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
		Renderer::GetDeviceContext()->CopySubresourceRegion(s_DataBuffer, 0, 0, 0, 0, resultBuffer, 0, nullptr);

		s_DataCount -= totalPad;

		for (unsigned int i = 0; i < s_Meta.size(); i++) {
			s_Meta[i].offset -= s_Meta[i].padding;
			s_Meta[i].padding = 0;
		}

		SAFE_RELEASE(copySRV);
		SAFE_RELEASE(metaSRV);
		SAFE_RELEASE(resultUAV);
		SAFE_RELEASE(copyBuffer);
		SAFE_RELEASE(metaBuffer);
		SAFE_RELEASE(resultBuffer);

	}

	void VertexDivision::Init()
	{
		if (!s_PadCS) {
			s_PadCS = Renderer::LoadComputeShader("complied_shader\\padVertexCS.cso");
		}
	}

	void VertexDivision::Uninit()
	{
		SAFE_RELEASE(s_DataBuffer);
		SAFE_RELEASE(s_PadCS);
		s_Meta.clear();
		s_EmptyIds.clear();
	}

	VertexDivision VertexDivision::Create(unsigned int count, const VERTEX* data) {
		VertexDivision key{};
		META meta{};
		meta.offset = s_DataCount;
		meta.count = count;

		if (s_DataCount + count > s_DataCapcity) {
			unsigned int newCapcity = max(s_DataCapcity + count * 2, DATA_INTERVAL);
			if (!ReserveData(newCapcity)) {
				return key;
			}
		}

		if (s_EmptyIds.empty()) {
			s_Meta.push_back(meta);
			key.m_Id = s_Meta.size() - 1;
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_Meta[key.m_Id] = meta;
		}

		// update data
		if (data) {
			D3D11_BOX box = Renderer::GetRangeBox(sizeof(VERTEX) * meta.offset, sizeof(VERTEX) * (meta.offset + meta.count));
			Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
		}
		s_DataCount += count;

		return key;
	}

	void VertexDivision::SetData(VERTEX* data) {
		META& meta = s_Meta[m_Id];
		D3D11_BOX box = Renderer::GetRangeBox(sizeof(VERTEX) * meta.offset, sizeof(VERTEX) * (meta.offset + meta.count));
		Renderer::GetDeviceContext()->UpdateSubresource(s_DataBuffer, 0, &box, data, 0, 0);
	}
}

