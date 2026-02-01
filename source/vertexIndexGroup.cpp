#include "vertexIndexGroup.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void VertexGroup::Init()
	{
		unsigned int newCapcity = max(ceil(static_cast<float>(s_Last) / DATA_INTERVAL), 1) * DATA_INTERVAL;

		ID3D11Buffer* newBuffer = nullptr;

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = sizeof(VERTEX) * newCapcity;
		desc.StructureByteStride = sizeof(VERTEX);
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;

		Renderer::GetDevice()->CreateBuffer(&desc, nullptr, &newBuffer);

		if (newBuffer && s_Buffer) {
			Renderer::GetDeviceContext()->CopyResource(newBuffer, s_Buffer);
			SAFE_RELEASE(s_SRV)
				SAFE_RELEASE(s_Buffer)
				s_SRV = nullptr;
			s_Buffer = nullptr;
		}

		s_Buffer = newBuffer;

		if (s_Buffer && s_SRV == nullptr) {
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.Buffer.NumElements = newCapcity;
			Renderer::GetDevice()->CreateShaderResourceView(s_Buffer, &srvDesc, &s_SRV);
		}
		if (s_Buffer && s_SRV) {
			s_Capcity = newCapcity;
		}
	}

	void VertexGroup::Uninit()
	{
		SAFE_RELEASE(s_SRV)
			SAFE_RELEASE(s_Buffer)
	}

	void VertexGroup::Update(const VERTEX* data)
	{
		if (s_Buffer && s_SRV) {
			D3D11_BOX box{};
			box.left = sizeof(VERTEX) * m_Id;
			box.right = sizeof(VERTEX) * m_Length;
			box.top = 0;
			box.bottom = 1;
			box.front = 0;
			box.back = 1;
			Renderer::GetDeviceContext()->UpdateSubresource(s_Buffer, 0, &box, data, 0, 0);

			Renderer::GetDeviceContext()->VSSetShaderResources(SLOT_VERTEX_BUFFER, 1, &s_SRV);
			Renderer::GetDeviceContext()->CSSetShaderResources(SLOT_VERTEX_BUFFER, 1, &s_SRV);
		}
	}
} // namespace MG