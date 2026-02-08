// =======================================================
// modelInstance.h
// 
// モデルインスタンスバッファ
// =======================================================
#pragma once
#include <vector>
#include <set>
#include "dynamicMatrix.h"
#include "model.h"
#include "material.h"
#include "dynamicIndexDivision.h"
#include "animationSet.h"
#include "matrixDivision.h"
#include "buffer.h"

namespace MG {
	class ModelInstance {
	public:
		struct DATA {
			Model model;
			DynamicIndexDivision materials;
			DynamicMatrix world;
			unsigned int enabled;
			AnimationSet animationSet;
			MatrixDivision matrixDivision;
			DynamicIndexDivision nodeParentIndexDivision;
			unsigned int lod = LOD_ALL;
		};

	private:
		static inline std::vector<DATA> s_Data;
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_Buffer = nullptr;
		static inline ID3D11ShaderResourceView* s_SRV = nullptr;
		static inline unsigned int s_Capcity = 0;
		static inline bool s_NeedUpdateBuffer = false;

	public:
		static ID3D11Buffer* const GetBuffer() { return s_Buffer; }
		static ID3D11ShaderResourceView* const GetSRV() { return s_SRV; }
		static size_t GetCount() { return s_Data.size(); }

		static ModelInstance Create(Model model = {}, DynamicMatrix world = {}, bool enabled = false, unsigned int lod = LOD_ALL)
		{
			ModelInstance key = {};
			DATA data{};
			data.model = model;
			auto& modelData = model.GetData();
			auto& materials = modelData.materials;
			std::vector<unsigned int> materialIds(materials.size());
			memcpy(materialIds.data(), materials.data(), sizeof(unsigned int) * materials.size());
			data.materials = DynamicIndexDivision::Create(static_cast<unsigned int>(materials.size()), materialIds.data());
			data.world = world;
			data.enabled = enabled;
			data.matrixDivision = modelData.originalNodeMatrixDivision;
			data.nodeParentIndexDivision = modelData.nodeParentIndexDivision;
			data.lod = lod;

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

		static void Uninit();
		static void Update();

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(ModelInstance)

		const Model& GetModel() const { return s_Data[m_Id].model; }
		const DynamicMatrix& GetWorld() const { return s_Data[m_Id].world; }
		bool IsEnabled() const { return s_Data[m_Id].enabled; }
		const DATA& GetData() const { return s_Data[m_Id]; }

		void SetModel(const Model model) 
		{
			s_Data[m_Id].model = model;
			s_NeedUpdateBuffer = true;
		}

		void SetWorld(const DynamicMatrix world) {
			s_Data[m_Id].world = world;
			s_NeedUpdateBuffer = true;
		}

		void SetMaterial(Material material, const unsigned int index) 
		{
			s_Data[m_Id].materials.SetData(index, material);
		}

		void SetEnabled(bool enabled) 
		{
			s_Data[m_Id].enabled = enabled;
			s_NeedUpdateBuffer = true;
		}

		void SetLOD(unsigned int lod) 
		{
			s_Data[m_Id].lod = lod;
			s_NeedUpdateBuffer = true;
		}

		void SetAnimationSet(AnimationSet animationSet) 
		{
			s_Data[m_Id].animationSet = animationSet;
			s_NeedUpdateBuffer = true;
		}

		void SetData(const DATA& data) 
		{
			s_Data[m_Id] = data;
			s_NeedUpdateBuffer = true;
		}

		void Release() 
		{
			if (m_Id != UINT_MAX) {
				s_Data[m_Id].materials.Release();
				s_Data[m_Id].animationSet.Release();
				s_Data[m_Id] = {};
				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
				s_NeedUpdateBuffer = true;
			}
		}
	};

}
