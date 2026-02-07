#pragma once
#include <vector>
#include <set>
#include "matrix4x4.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

namespace MG {
	class MatrixDivsion {
		struct META {
			unsigned int offset;
			unsigned int count;
			unsigned int enabled;
		};
	private:
		static inline std::vector<META> s_Meta{};
		static inline std::set<unsigned int> s_EmptyIds{};

		static inline ID3D11Buffer* s_MetaBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_MetaSRV = nullptr;
		static inline ID3D11Buffer* s_DataBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_DataSRV = nullptr;
		static inline unsigned int s_Capcity = 0;
		static inline unsigned int s_Last = 0;
		static const constexpr unsigned int DATA_INTERVAL = 5000;

	public:
		static void Reserve(unsigned int newCapcity);
		static void Pad();
		static void Fit();
		static void Uninit();

		static MatrixDivsion Create(unsigned int count, const Matrix4x4* data = nullptr) {
			MatrixDivsion key{};
			META meta{};

			if (s_Last + count > s_Capcity) {
				unsigned int newCapcity = max(s_Capcity + count * 2, DATA_INTERVAL);
				Reserve(newCapcity);
			}

			if (s_EmptyIds.empty()) {
				s_Meta.push_back(meta);
				key.m_Id = s_Meta.size() - 1;
			}
			else {
				auto it = std::next(s_EmptyIds.begin(), 1);
				key.m_Id = *it;
				s_EmptyIds.erase(it);
				s_Meta[key.m_Id] = meta;
			}

			if (s_Data.size() > s_Capcity) {
				Resize();
			}

			s_NeedUpdateBuffer = true;
			return key;
		}

		static MatrixGroup Create(const Matrix4x4* data, unsigned int count) {

			MatrixGroup key = {};
			key.m_Id = s_Last;
			key.m_Length = count;
			s_Last += count;

			if (s_Last > s_Capcity) {
				Resize();
			}
			// “]‘—
			key.Update(data);

			s_Registered[key.m_Id] = key;
			return key;
		}
	private:
		unsigned int m_Id = UINT_MAX;

		void Update(const Matrix4x4* data);
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

