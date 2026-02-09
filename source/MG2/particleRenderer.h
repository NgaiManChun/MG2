#pragma once
#include "component.h"
#include "material.h"
#include "matrixDivision.h"
#include "vectorDivision.h"
#include "renderer.h"
#include "timeLine.h"
#include <unordered_map>

struct ID3D11UnorderedAccessView;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11ComputeShader;

namespace MG {


	class ParticleRenderer : public Component {
	private:
		static inline std::unordered_map<std::string, ID3D11ComputeShader*> s_CSPool;
		static inline std::vector<DRAW_INDIRECT_ARGS> s_DrawArgs;
		static inline std::set<unsigned int> s_EmptyIds;
		static inline ID3D11Buffer* s_DrawArgsBuffer = nullptr;
		static inline ID3D11UnorderedAccessView* s_DrawArgsUAV = nullptr;
		static inline ID3D11ShaderResourceView* s_DrawArgsSRV = nullptr;
		static inline ID3D11Buffer* s_DrawArgsIndirectBuffer = nullptr;
		static inline ID3D11Buffer* s_SpawnCountBuffer = nullptr;
		static inline ID3D11UnorderedAccessView* s_SpawnCountUAV = nullptr;
		static inline unsigned int s_Capcity = 0;

	private:
		Material m_Material;
		ID3D11Buffer* m_DataBuffer;
		ID3D11UnorderedAccessView* m_DataUAV;
		ID3D11ShaderResourceView* m_DataSRV;
		ID3D11Buffer* m_IndexBuffer;
		ID3D11UnorderedAccessView* m_IndexUAV;
		ID3D11ShaderResourceView* m_IndexSRV;
		ID3D11ComputeShader* m_UpdateCS;
		unsigned int m_Count;
		PARTICLE_CONSTANT m_Constant;
		TimeLine m_SpawnTimeLine{ 0.3f };
		unsigned int m_SpawnCount = 5;
		unsigned int m_ArgsIndex = UINT_MAX;
	public:
		ParticleRenderer(Material material, unsigned int count, const char* updateCSFilename);
		void Init() override;
		void Uninit() override;
		const PARTICLE_CONSTANT& GetConstant()
		{
			return m_Constant;
		}
		void SetConstant(PARTICLE_CONSTANT constant)
		{
			m_Constant = constant;
		}
		void SetSpawnTimeLine(TimeLine timeLine)
		{
			m_SpawnTimeLine = timeLine;
		}
		void SetSpawnCount(unsigned int spawnCount)
		{
			m_SpawnCount = spawnCount;
		}

		static void StaticInit();
		static void StaticUninit();
		static void UpdateAll(Scene* scene);
		static void DrawAll(Scene* scene);
	private:
		BIND_STATIC_INIT(ParticleRenderer, ParticleRenderer::StaticInit)
		BIND_STATIC_UNINIT(ParticleRenderer, ParticleRenderer::StaticUninit)
		BIND_UPDATE_ALL(ParticleRenderer, ParticleRenderer::UpdateAll)
		BIND_DRAW_ALL(ParticleRenderer, ParticleRenderer::DrawAll, 40)
		BIND_COMPONENT(ParticleRenderer)

	};
} // namespace MG

