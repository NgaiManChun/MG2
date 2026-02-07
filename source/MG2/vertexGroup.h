#pragma once
#include <vector>
#include <map>
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

namespace MG {
	class VertexGroup {
		struct VERTEX
		{
			Vector3 position;
			Vector3 normal;
			Vector3 tangent;
			Vector3 bitangent;
			Vector2 texCoord;
			RGBA color;
		};
	private:
		static inline std::map<unsigned int, VertexGroup> s_Registered{};

		static inline ID3D11Buffer* s_Buffer = nullptr;
		static inline ID3D11ShaderResourceView* s_SRV = nullptr;
		static inline unsigned int s_Capcity = 0;
		static inline unsigned int s_Last = 0;
		static const constexpr unsigned int DATA_INTERVAL = 500000;

	public:
		static void Init();
		static void Uninit();

		static VertexGroup Create(const VERTEX* data, unsigned int size) {

			VertexGroup key = {};
			key.m_Id = s_Last;
			key.m_Length = size;
			s_Last += size;

			if (s_Last > s_Capcity) {
				// Šg‘å
				Init();
			}
			// “]‘—
			key.Update(data);

			s_Registered[key.m_Id] = key;
			return key;
		}
	private:
		unsigned int m_Id = UINT_MAX;
		unsigned int m_Length;

		void Update(const VERTEX* data);
	public:

		void Release() {
			if (m_Id != UINT_MAX) {
				s_Registered.erase(m_Id);
				m_Id = UINT_MAX;
				m_Length = 0;
			}
		}

		operator bool() {
			return m_Id != UINT_MAX;
		}
	};

}

