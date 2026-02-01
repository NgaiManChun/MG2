#pragma once
#include "directX.h"
#include "dataType.h"
#include <unordered_map>
#include <string>

namespace MG {

	struct CAMERA_CONSTANT
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		XMFLOAT4X4 viewProjection;
		XMFLOAT4X4 invViewProjection;
		XMFLOAT4X4 invViewRotation;
		XMFLOAT4 frustum[4];
		XMFLOAT3 cameraPosition;
		float projectionFar;
		UINT forceLOD;
		XMFLOAT3 cameraForward;
		XMFLOAT4 frustumPoints[4];
		//UINT flags[2];
	};

	struct LIGHT_CONSTANT
	{
		Vector4 ambient;
		Vector4 directLightColor;
		Vector4 directLightDirection;
		XMFLOAT4X4 directionalShadowViewProjection;
	};

	struct MODEL_CONSTANT
	{
		UINT modelId;
		UINT modelMaterialCount;
		UINT nodeCount;
		UINT maxInstance;
		UINT nodeMatrixDivisionId;
		UINT nodeParentIndexDivisionOffset;
		UINT flags[2];
	};

	struct MESH_CONSTANT
	{
		UINT meshId;
		UINT meshMaterialOffset;
		UINT nodeIndex;
		UINT skinning;
		UINT boneDivisionOffset;
		UINT vertexBoneWeightDivisionOffset;
		Vector3 localMin;
		Vector3 localMax;
	};

	struct CS_CONSTANT
	{
		UINT CSMaxX;
		UINT CSMaxY;
		UINT CSMaxZ;
		UINT CSMaxW;
	};

	struct SINGLE_CONSTANT {
		UINT worldMatrixId;
		UINT localMatrixDivisionOffset;
		UINT materialId;
		UINT flags[1];
	};

	struct PARTICLE_CONSTANT {
		Vector4 INIT_POSITION_RANGE = Vector3(5.0f, 5.0f, 5.0f);
		Vector4 INIT_SCALE = Vector3(0.3f, 0.3f, 0.3f);
		Vector4 INIT_SCALE_ACCELERATION = Vector3(0.0f, 0.0f, 0.0f);
		Vector4 INIT_VELOCITY = Vector3(1.0f, 1.0f, 1.0f);
		Vector4 INIT_ACCELERATION = Vector3(0.1f, 0.1f, 0.1f);
		Vector4 INIT_COLOR = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		float INIT_LIFE_MIN = 5.0f;
		float INIT_LIFE_MAX = 15.0f;
		float FADE_IN_TIME = 1.0f;
		float FADE_OUT_TIME = 1.0f;
	};
	
	struct TIME_CONSTANT
	{
		UINT currentTime;
		UINT deltaTime;
		UINT flags[2];
	};

	enum BLEND_STATE
	{
		BLEND_STATE_NONE = 0,	// ブレンドしない
		BLEND_STATE_ALPHA_A2C,	// αブレンド + AlphaToCoverage
		BLEND_STATE_ALPHA,		// 普通のαブレンド
		BLEND_STATE_ADD,		// 加算合成
		BLEND_STATE_SUB,		// 減算合成
		BLEND_STATE_DEST_OUT,	// くりぬき
		BLEND_STATE_MAX
	};

	enum DEPTH_STATE
	{
		DEPTH_STATE_COMPARISON_LESS = 0,
		DEPTH_STATE_COMPARISON_ALWAYS,
		DEPTH_STATE_NO_WRITE_COMPARISON_LESS,
		DEPTH_STATE_NO_WRITE_COMPARISON_ALWAYS,
		DEPTH_STATE_MAX
	};

	enum RASTERIZER_STATE
	{
		RASTERIZER_STATE_SOLID_CULL_BACK = 0,
		RASTERIZER_STATE_SOLID_CULL_FRONT,
		RASTERIZER_STATE_SOLID_CULL_NONE,
		RASTERIZER_STATE_WIREFRAME_CULL_NONE,
		RASTERIZER_STATE_MAX
	};

	struct SHADER_SET {
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;
		ID3D11ComputeShader* computeShader;
		ID3D11InputLayout* inputLayout;
	};

	enum SHADER_TYPE {
		SHADER_TYPE_TEXTURE_COPY,
		SHADER_TYPE_UNLIT,
		SHADER_TYPE_SPOT_LIGHT,
		SHADER_TYPE_DEFERRED_LIGHT
	};

	


	typedef unsigned int TEXTURE_SLOT;

	
	// カスタムノードアトリビュート(instance)
	const constexpr unsigned int INSTANCE_TYPE_NONE = (0);
	const constexpr unsigned int INSTANCE_TYPE_COLLISION_POINT = (0xFFFF);
	const constexpr unsigned int INSTANCE_TYPE_COLLISION_AABB = (0xFFFE);
	const constexpr unsigned int INSTANCE_TYPE_COLLISION_SPHERE = (0xFFFD);
	const constexpr unsigned int INSTANCE_TYPE_COLLISION_BOX = (0xFFFC);
	const constexpr unsigned int INSTANCE_TYPE_COLLISION_CAPSULE = (0xFFFB);

	const constexpr unsigned int SLOT_DYNAMIC_MATRIX = 0;
	const constexpr unsigned int SLOT_MATERIAL = 1;
	const constexpr unsigned int SLOT_MODEL_INSTANCE = 2;
	const constexpr unsigned int SLOT_MODEL_INSTANCE_MATERIALS = 3;
	const constexpr unsigned int SLOT_NODE_MATRIX = 4;
	const constexpr unsigned int SLOT_MESH_INSTANCE = 5;

	const constexpr unsigned int SLOT_TEXTURE = 10;

	class Renderer
	{
	private:

		static inline D3D_FEATURE_LEVEL s_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

		static inline ID3D11Device* s_Device = nullptr;
		static inline ID3D11DeviceContext* s_DeviceContext = nullptr;
		static inline IDXGISwapChain* s_SwapChain = nullptr;
		static inline ID3D11RenderTargetView* s_RenderTargetView = nullptr;
		static inline ID3D11DepthStencilView* s_DepthStencilView = nullptr;

		// 定数バッファ
		static inline ID3D11Buffer* s_CameraConstantBuffer = nullptr;
		static inline ID3D11Buffer* s_LightConstantBuffer = nullptr;
		static inline ID3D11Buffer* s_ModelConstantBuffer = nullptr;
		static inline ID3D11Buffer* s_MeshConstantBuffer = nullptr;
		static inline ID3D11Buffer* s_CSConstantBuffer = nullptr;
		static inline ID3D11Buffer* s_SingleConstantBuffer = nullptr;
		static inline ID3D11Buffer* s_ParticleConstantBuffer = nullptr;
		static inline ID3D11Buffer* s_TimeConstantBuffer = nullptr;
		

		// 各種ステート
		static inline ID3D11DepthStencilState* s_DepthStates[DEPTH_STATE_MAX];
		static inline ID3D11BlendState* s_BlendStates[BLEND_STATE_MAX];
		static inline ID3D11RasterizerState* s_RasterizerStates[RASTERIZER_STATE_MAX];

		static inline std::unordered_map<SHADER_TYPE, SHADER_SET> s_Shaders{};
		
		static inline D3D11_INPUT_ELEMENT_DESC s_Layout[] =
		{
			{ "MESHID",			0, DXGI_FORMAT_R32_UINT, 0,	 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "MODELINSTANCEID",0, DXGI_FORMAT_R32_UINT, 0,	 4, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "LOCALID",		0, DXGI_FORMAT_R32_UINT, 0,	12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "MATERIALID",		0, DXGI_FORMAT_R32_UINT, 0,	16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};

	public:

		static void Init(HWND hWnd);
		static void Uninit();
		static void Begin();
		static void End();

		static void SetViewport(float width, float height);
		static void Apply2D(float viewWidth, float viewHeight);
		static void SetDepthState(DEPTH_STATE state);
		static void SetBlendState(BLEND_STATE state);
		static void SetRasterizerState(RASTERIZER_STATE state);
		static void SetPrimitiveTopology(PRIMITIVE_TYPE type);

		// 定数バッファ
		static void SetCamera(const CAMERA_CONSTANT& constant);
		static void SetLight(const LIGHT_CONSTANT& constant);
		static void SetModelContant(const MODEL_CONSTANT& constant);
		static void SetMeshContant(const MESH_CONSTANT& constant);
		static void SetCSContant(const CS_CONSTANT& constant);
		static void SetSingleContant(const SINGLE_CONSTANT& constant);
		static void SetParticleContant(const PARTICLE_CONSTANT& constant);
		static void SetTimeContant(const TIME_CONSTANT& constant);
		

		static ID3D11Device* GetDevice(void) { return s_Device; }
		static ID3D11DeviceContext* GetDeviceContext(void) { return s_DeviceContext; }
		static ID3D11RenderTargetView* GetMainRenderTarget() { return s_RenderTargetView; }
		static ID3D11DepthStencilView* GetMainDepthStencilView() { return s_DepthStencilView; }

		static D3D11_BOX GetRangeBox(unsigned int left, unsigned int right)
		{
			D3D11_BOX box{};
			box.left = left;
			box.right = right;
			box.top = 0;
			box.bottom = 1;
			box.front = 0;
			box.back = 1;
			return box;
		}

		static ID3D11Buffer* CreateVertexBuffer(unsigned int byteWidth);
		static ID3D11Buffer* CreateStructuredBuffer(unsigned int stride, unsigned int count, const void* data = nullptr, unsigned int bindFlag = D3D11_BIND_SHADER_RESOURCE);
		static ID3D11ShaderResourceView* CreateStructuredSRV(ID3D11Buffer* buffer, unsigned int elementCount);
		static ID3D11UnorderedAccessView* CreateStructuredUAV(ID3D11Buffer* buffer, unsigned int elementCount, unsigned int flags = 0);

		static ID3D11Texture2D* CreateTexture2D(
			unsigned int width, unsigned int height,
			DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			unsigned int bindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE
		);
		static ID3D11RenderTargetView* CreateTextureRTV(ID3D11Texture2D* texture, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		static ID3D11ShaderResourceView* CreateTextureSRV(ID3D11Texture2D* texture, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		static ID3D11DepthStencilView* CreateTextureDSV(ID3D11Texture2D* texture, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT);
		

		static SHADER_SET GetShaderSet(SHADER_TYPE type) { return s_Shaders[type]; }
		static SHADER_SET LoadVertexShader(const char* filename, D3D11_INPUT_ELEMENT_DESC* layout, UINT numElements);
		static ID3D11PixelShader* LoadPixelShader(const char* filename);
		static ID3D11ComputeShader* LoadComputeShader(const char* filename);
		static ID3D11GeometryShader* LoadGeometryShader(const char* filename);

		static void SetMainRenderTarget();

	};
} // namespace MG


