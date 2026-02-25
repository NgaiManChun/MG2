#include "mesh.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void Mesh::Update()
	{
		if (s_NeedUpdateBuffer) {

			unsigned int newCapacity = static_cast<unsigned int>(s_DrawArgs.capacity());
			if (newCapacity > s_DrawArgsBufferCapacity) {
				SAFE_RELEASE(s_DrawArgsSRV);
				SAFE_RELEASE(s_DrawArgsUAV);
				SAFE_RELEASE(s_DrawArgsBuffer);
				SAFE_RELEASE(s_DrawArgsIndirectBuffer);

				s_DrawArgsBuffer = Renderer::CreateStructuredBuffer(sizeof(DRAW_INDEXED_INDIRECT_ARGS), newCapacity, s_DrawArgs.data(), D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
				s_DrawArgsUAV = Renderer::CreateStructuredUAV(s_DrawArgsBuffer, newCapacity);
				s_DrawArgsSRV = Renderer::CreateStructuredSRV(s_DrawArgsBuffer, newCapacity);

				D3D11_BUFFER_DESC desc = {};
				desc.ByteWidth = sizeof(DRAW_INDEXED_INDIRECT_ARGS) * newCapacity;
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = 0;
				desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
				D3D11_SUBRESOURCE_DATA subResourceData = {};
				subResourceData.SysMemPitch = 0;
				subResourceData.SysMemSlicePitch = 0;
				subResourceData.pSysMem = s_DrawArgs.data();
				Renderer::GetDevice()->CreateBuffer(&desc, &subResourceData, &s_DrawArgsIndirectBuffer);
				s_DrawArgsBufferCapacity = newCapacity;
				s_NeedUpdateBuffer = false;
			}
		}

		if (s_DrawArgsBuffer) {
			D3D11_BOX box = Renderer::GetRangeBox(0, static_cast<unsigned int>(sizeof(DRAW_INDEXED_INDIRECT_ARGS) * s_DrawArgs.size()));
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
		s_DrawArgsBufferCapacity = 0;
		s_NeedUpdateBuffer = false;
	}

} // namespace MG
