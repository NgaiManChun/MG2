// =======================================================
// modelAnimation.h
// 
// モデルに適用したアニメーションのバッファ
// =======================================================
#pragma once
#include <vector>
#include <set>
#include "transformDivision.h"
#include "buffer.h"

namespace MG {
	class ModelAnimation {
	public:
		struct DATA {
			unsigned int modelId;
			unsigned int animationId;
			TransformDivision transformDivision;
			unsigned int nodeCount;
			unsigned int frameCount;
			unsigned int duration;
			unsigned int loop;
		};

	private:
		static inline std::vector<DATA> s_Data;
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_Buffer = nullptr;
		static inline ID3D11ShaderResourceView* s_SRV = nullptr;
		static inline unsigned int s_Capcity = 0;
		static inline bool s_NeedUpdateBuffer = false;

	public:
		static ID3D11ShaderResourceView* GetSRV() { return s_SRV; }

		static void Uninit();
		static void Update();
		static ModelAnimation Create(unsigned int modelId, unsigned int animationId, bool loop = false);

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(ModelAnimation)

		const DATA& GetData() const { return s_Data[m_Id]; }

		void Release() 
		{
			if (m_Id != UINT_MAX) {
				s_Data[m_Id].transformDivision.Release();
				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}

	};

}
