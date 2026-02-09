#include "animationSet.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void AnimationSet::Uninit()
	{
		SAFE_RELEASE(s_SRV);
		SAFE_RELEASE(s_Buffer);
		SAFE_RELEASE(s_ResultSRV);
		SAFE_RELEASE(s_ResultUAV);
		SAFE_RELEASE(s_ResultBuffer);
		s_Data.clear();
		s_EmptyIds.clear();
		s_Capcity = 0;
		s_NeedUpdateBuffer = false;
	}

	void AnimationSet::Update()
	{
		if (s_NeedUpdateBuffer) {

			// バッファ確保
			unsigned int newCapcity = static_cast<unsigned int>(s_Data.capacity());
			if (newCapcity > s_Capcity) {
				SAFE_RELEASE(s_SRV);
				SAFE_RELEASE(s_Buffer);
				SAFE_RELEASE(s_ResultSRV);
				SAFE_RELEASE(s_ResultUAV);
				SAFE_RELEASE(s_ResultBuffer);

				s_Buffer = Renderer::CreateStructuredBuffer(sizeof(DATA), newCapcity, s_Data.data());
				if (s_Buffer) {
					s_SRV = Renderer::CreateStructuredSRV(s_Buffer, newCapcity);
					s_Capcity = newCapcity;
					s_NeedUpdateBuffer = false;
				}

				s_ResultBuffer = Renderer::CreateStructuredBuffer(RESULT_STRIDE, newCapcity, nullptr,
					D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
				if (s_ResultBuffer) {
					s_ResultSRV = Renderer::CreateStructuredSRV(s_ResultBuffer, newCapcity);
					s_ResultUAV = Renderer::CreateStructuredUAV(s_ResultBuffer, newCapcity);
				}
			}
		}

		if (s_NeedUpdateBuffer && s_SRV) {
			D3D11_BOX box = Renderer::GetRangeBox(0, static_cast<unsigned int>(sizeof(DATA) * s_Data.size()));
			Renderer::GetDeviceContext()->UpdateSubresource(s_Buffer, 0, &box, s_Data.data(), 0, 0);
			s_NeedUpdateBuffer = false;
		}
	}

	void AnimationSet::Swap(ModelAnimation nextAnimation, unsigned int blendDuration, unsigned int timeOffset) {
		DATA& animationSetData = s_Data[m_Id];
		unsigned int nowTime = MGUtility::GetRunTimeMilliseconds();
		unsigned int startTime = nowTime - timeOffset;
		
		animationSetData.modelAnimationsFrom[0] = animationSetData.modelAnimationsTo[0];
		animationSetData.animationStartTimeFrom[0] = animationSetData.animationStartTimeTo[0];
		animationSetData.countFrom = animationSetData.countTo;

		animationSetData.modelAnimationsTo[0] = nextAnimation;
		animationSetData.animationStartTimeTo[0] = startTime;
		animationSetData.countTo = 1;

		unsigned int blendOffset = animationSetData.animationBlendDuration - min(nowTime - animationSetData.animationBlendStartTime, animationSetData.animationBlendDuration);

		animationSetData.animationBlendDuration = blendDuration;
		animationSetData.animationBlendStartTime = startTime - blendOffset;
		s_NeedUpdateBuffer = true;
	}

} // namespace MG
