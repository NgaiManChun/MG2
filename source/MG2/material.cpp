#include "material.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void Material::Uninit()
	{
		SAFE_RELEASE(s_SRV)
		SAFE_RELEASE(s_Buffer)
		s_Data.clear();
		s_EmptyIds.clear();
		s_Capacity = 0;
		s_NeedUpdateBuffer = false;
	}

	void Material::Update()
	{
		if (s_NeedUpdateBuffer) {

			// バッファ確保
			unsigned int newCapacity = static_cast<unsigned int>(s_Data.capacity());
			if (newCapacity > s_Capacity) {
				SAFE_RELEASE(s_SRV);
				SAFE_RELEASE(s_Buffer);
				s_Buffer = Renderer::CreateStructuredBuffer(sizeof(MATERIAL), newCapacity, s_Data.data());
				if (s_Buffer) {
					s_SRV = Renderer::CreateStructuredSRV(s_Buffer, newCapacity);
					s_Capacity = newCapacity;
					s_NeedUpdateBuffer = false;
				}
			}
		}

		if (s_NeedUpdateBuffer && s_SRV) {
			D3D11_BOX box = Renderer::GetRangeBox(0, static_cast<unsigned int>(sizeof(MATERIAL) * s_Data.size()));
			Renderer::GetDeviceContext()->UpdateSubresource(s_Buffer, 0, &box, s_Data.data(), 0, 0);
			s_NeedUpdateBuffer = false;
		}
	}

} // namespace MG
