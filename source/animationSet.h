#pragma once
#include <vector>
#include <set>
#include "dataType.h"
#include "modelAnimation.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

namespace MG {
	class AnimationSet {
	public:
		struct DATA {
			ModelAnimation modelAnimationsFrom[1];
			unsigned int animationStartTimeFrom[1];
			unsigned int countFrom;
			ModelAnimation modelAnimationsTo[1];
			unsigned int animationStartTimeTo[1];
			unsigned int countTo;
			unsigned int animationBlendDuration;
			unsigned int animationBlendStartTime;
			float timeMultiplier = 1.0f;
		};

		struct RESULT {
			// GPU—p
			unsigned int nodeParentOffset;
			unsigned int transformOffsetFrom;
			unsigned int transformOffsetTo;
			float blend;
		};

	private:
		static inline std::vector<DATA> s_Data;
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_Buffer = nullptr;
		static inline ID3D11ShaderResourceView* s_SRV = nullptr;
		static inline ID3D11Buffer* s_ResultBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_ResultSRV = nullptr;
		static inline ID3D11UnorderedAccessView* s_ResultUAV = nullptr;
		static inline unsigned int s_Capcity = 0;
		static inline bool s_NeedUpdateBuffer = false;
	public:
		static ID3D11ShaderResourceView* GetSRV() { return s_SRV; }
		static ID3D11Buffer* GetResultBuffer() { return s_ResultBuffer; }
		static ID3D11ShaderResourceView* GetResultSRV() { return s_ResultSRV; }
		static ID3D11UnorderedAccessView* GetResultUAV() { return s_ResultUAV; }
		static void Uninit();
		static void Update();
		static AnimationSet Create(AnimationSet::DATA data)
		{
			AnimationSet key = {};
			if (s_EmptyIds.empty()) {
				s_Data.push_back(data);
				key.m_Id = static_cast<unsigned int>(s_Data.size()) - 1;
			}
			else {
				key.m_Id = *s_EmptyIds.begin();
				s_EmptyIds.erase(s_EmptyIds.begin());
				s_Data[key.m_Id] = data;
			}

			s_NeedUpdateBuffer = true;
			return key;
		}
	private:
		unsigned int m_Id = UINT_MAX;
	public:

		const DATA& GetData() const { return s_Data[m_Id]; }

		void SetData(AnimationSet::DATA data) 
		{ 
			s_Data[m_Id] = data; 
			s_NeedUpdateBuffer = true;
		}

		void Swap(ModelAnimation nextAnimation, unsigned int blendDuration, unsigned int timeOffset = 0);

		void Release() {
			if (m_Id != UINT_MAX) {
				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}

		operator bool() const {
			return m_Id != UINT_MAX;
		}

		operator unsigned int() const {
			return m_Id;
		}

	};

}
