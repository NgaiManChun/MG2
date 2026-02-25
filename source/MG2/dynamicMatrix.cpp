#include "dynamicMatrix.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void DynamicMatrix::Uninit()
	{
		SAFE_RELEASE(s_SRV);
		SAFE_RELEASE(s_UAV);
		SAFE_RELEASE(s_Buffer);
		s_Data.clear();
		s_EmptyIds.clear();
		s_Capacity = 0;
		s_NeedUpdateBuffer = false;
	}

	void DynamicMatrix::Update()
	{
		if (s_NeedUpdateBuffer) {

			// バッファ確保
			unsigned int newCapacity = static_cast<unsigned int>(s_Data.capacity());
			if (newCapacity > s_Capacity) {
				SAFE_RELEASE(s_SRV);
				SAFE_RELEASE(s_UAV);
				SAFE_RELEASE(s_Buffer);
				s_Buffer = Renderer::CreateStructuredBuffer(
					sizeof(Matrix4x4), static_cast<unsigned int>(newCapacity), s_Data.data(),
					D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
				if (s_Buffer) {
					s_SRV = Renderer::CreateStructuredSRV(s_Buffer, newCapacity);
					s_UAV = Renderer::CreateStructuredUAV(s_Buffer, newCapacity);
					s_Capacity = newCapacity;
					s_NeedUpdateBuffer = false;
				}
			}
		}

		if (s_NeedUpdateBuffer && s_SRV) {
			D3D11_BOX box = Renderer::GetRangeBox(0, static_cast<unsigned int>(sizeof(Matrix4x4) * s_Data.size()));
			Renderer::GetDeviceContext()->UpdateSubresource(s_Buffer, 0, &box, s_Data.data(), 0, 0);
			s_NeedUpdateBuffer = false;
		}
	}
} // namespace MG
