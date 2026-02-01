#include "mesh.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void Mesh::Update()
	{
		if (s_NeedUpdateBuffer) {
			if (s_DrawArgs.capacity() > s_DrawArgsBufferCapcity) {
				SAFE_RELEASE(s_DrawArgsSRV);
				SAFE_RELEASE(s_DrawArgsUAV);
				SAFE_RELEASE(s_DrawArgsBuffer);
				SAFE_RELEASE(s_DrawArgsIndirectBuffer);

				s_DrawArgsBuffer = Renderer::CreateStructuredBuffer(sizeof(DRAW_INDEXED_INDIRECT_ARGS), s_DrawArgs.capacity(), s_DrawArgs.data(), D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
				s_DrawArgsUAV = Renderer::CreateStructuredUAV(s_DrawArgsBuffer, s_DrawArgs.capacity());
				s_DrawArgsSRV = Renderer::CreateStructuredSRV(s_DrawArgsBuffer, s_DrawArgs.capacity());

				D3D11_BUFFER_DESC desc = {};
				desc.ByteWidth = sizeof(DRAW_INDEXED_INDIRECT_ARGS) * s_DrawArgs.capacity();
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = 0;
				desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
				D3D11_SUBRESOURCE_DATA subResourceData = {};
				subResourceData.SysMemPitch = 0;
				subResourceData.SysMemSlicePitch = 0;
				subResourceData.pSysMem = s_DrawArgs.data();
				Renderer::GetDevice()->CreateBuffer(&desc, &subResourceData, &s_DrawArgsIndirectBuffer);

				

				/*D3D11_BUFFER_DESC desc = {};
				desc.ByteWidth = sizeof(DRAW_INDEXED_INDIRECT_ARGS) * s_DrawArgs.capacity();
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
				desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS | D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
				D3D11_SUBRESOURCE_DATA subResourceData = {};
				subResourceData.SysMemPitch = 0;
				subResourceData.SysMemSlicePitch = 0;
				subResourceData.pSysMem = s_DrawArgs.data();
				Renderer::GetDevice()->CreateBuffer(&desc, &subResourceData, &s_DrawArgsBuffer);
				if (s_DrawArgsBuffer) {
					D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
					uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
					uavDesc.Buffer.FirstElement = 0;
					uavDesc.Buffer.NumElements = sizeof(DRAW_INDEXED_INDIRECT_ARGS) / 4 * s_DrawArgs.capacity();
					uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
					Renderer::GetDevice()->CreateUnorderedAccessView(s_DrawArgsBuffer, &uavDesc, &s_DrawArgsUAV);
					s_DrawArgsBufferCapcity = s_Data.capacity();
					s_NeedUpdateBuffer = false;
				}*/
			}
		}
		if (s_DrawArgsBuffer) {
			//D3D11_BOX box = Renderer::GetRangeBox(0, sizeof(DRAW_INDEXED_INDIRECT_ARGS) * s_DrawArgs.size());
			D3D11_BOX box = Renderer::GetRangeBox(0, sizeof(DRAW_INDEXED_INDIRECT_ARGS) * s_DrawArgs.size());
			Renderer::GetDeviceContext()->UpdateSubresource(s_DrawArgsBuffer, 0, &box, s_DrawArgs.data(), 0, 0);
			s_NeedUpdateBuffer = false;
		}
	}

	void Mesh::Uninit()
	{
		SAFE_RELEASE(s_DrawArgsSRV);
		SAFE_RELEASE(s_DrawArgsUAV);
		SAFE_RELEASE(s_DrawArgsBuffer);
		SAFE_RELEASE(s_DrawArgsIndirectBuffer);
		for (auto& data : s_Data) {
			data.vertexDivision.Release();
			data.vertexIndexDivision.Release();
		}
		s_Data.clear();
		s_DrawArgs.clear();
		s_EmptyIds.clear();
	}

} // namespace MG
