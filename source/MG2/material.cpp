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
		s_Capcity = 0;
		s_NeedUpdateBuffer = false;
	}

	void Material::Update()
	{
		if (s_NeedUpdateBuffer) {

			// バッファ確保
			if (s_Data.capacity() > s_Capcity) {
				SAFE_RELEASE(s_SRV);
				SAFE_RELEASE(s_Buffer);
				s_Buffer = Renderer::CreateStructuredBuffer(sizeof(MATERIAL), s_Data.capacity(), s_Data.data());
				if (s_Buffer) {
					s_SRV = Renderer::CreateStructuredSRV(s_Buffer, s_Data.capacity());
					s_Capcity = s_Data.capacity();
					s_NeedUpdateBuffer = false;
				}
			}
		}

		if (s_NeedUpdateBuffer && s_SRV) {
			D3D11_BOX box = Renderer::GetRangeBox(0, sizeof(MATERIAL) * s_Data.size());
			Renderer::GetDeviceContext()->UpdateSubresource(s_Buffer, 0, &box, s_Data.data(), 0, 0);
			s_NeedUpdateBuffer = false;
		}
	}

} // namespace MG
