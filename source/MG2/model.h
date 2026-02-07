#pragma once
#include <vector>
#include <set>
#include <array>
#include "dataType.h"
#include "dynamicMatrix.h"
#include "texture.h"
#include "material.h"
#include "mesh.h"
#include "animation.h"
#include "modelAnimation.h"
#include "matrixDivision.h"
#include "dynamicIndexDivision.h"
#include "transformDivision.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

namespace MG {
	class Model {
	public:
		/*struct ANIMATION_SET {
			unsigned int animationIndexes[1];
			unsigned int animationStartTime[1];
		};*/
		//struct MODEL_INSTANCE {
		//	DynamicMatrix world;
		//	unsigned int enabled;
		//	unsigned int animation;

		//	//ANIMATION_SET animationSet0;
		//	//ANIMATION_SET animationSet1;
		//	//unsigned int animationBlendDuration;
		//	//unsigned int animationBlendStartTime;
		//	//float timeMultiplier = 1.0f;
		//};
		/*struct MODEL_ANIMATION {
			Animation animation;
			unsigned int frameCount;
			unsigned int duration;
			unsigned int transformOffset;
		};*/
		struct DATA {
			unsigned int nodeCount;
			std::unordered_map<std::string, unsigned int> namedNodeIndexes;
			std::vector<Texture> textures;
			std::vector<Material> materials;
			std::vector<Mesh> meshes;
			std::vector<NODE_MESH_PAIR> nodeMeshPairs;

			MatrixDivision originalNodeMatrixDivision;
			DynamicIndexDivision nodeParentIndexDivision;

			std::array<ModelAnimation, UCHAR_MAX> animations;

			/*std::vector<ANIMATION_META> animationMetaArray;
			bool animationNeedUpdate = false;
			unsigned int animationCapcity = 0;
			unsigned int animationTransformCapcity = 0;*/
		};
		
	private:
		static inline std::vector<DATA> s_Data;
		static inline std::unordered_map<std::string, unsigned int> s_NameMap;
		static inline std::set<unsigned int> s_EmptyIds{};

		static const constexpr unsigned int ANIMATION_INTERVAL = 10;
	public:
		static void Uninit();

		static Model Create(const char* filename);
	private:
		unsigned int m_Id = UINT_MAX;
	public:

		DATA& GetData() const { return s_Data[m_Id]; }

		unsigned int GetNodeCount() const { return s_Data[m_Id].nodeCount; }

		ModelAnimation GetModelAnimation(unsigned char slot) const { return s_Data[m_Id].animations[slot]; }

		void Release();

		void BindAnimation(unsigned char slot, const Animation animation, bool loop = false);

		operator bool() const {
			return m_Id != UINT_MAX;
		}

		operator unsigned int() const {
			return m_Id;
		}

		bool operator ==(Model& other) const {
			return m_Id == other.m_Id;
		}

		Model& operator=(const unsigned int& id) { m_Id = id; }
		Model() = default;
		Model(const unsigned int& id) :m_Id(id) {}
	};

}
