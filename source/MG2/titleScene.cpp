#include "scene.h"
#include "camera.h"
#include "particleRenderer.h"
#include "modelRenderer.h"
#include "input.h"
#include "MGUtility.h"

using namespace MG;

class TitleScene : public Scene
{
private:
	REGISTER_SCENE(TitleScene)
private:
	GameObject* m_Lamp;
	ModelRenderer* m_LampModelRenderer;
	ParticleRenderer* flog;
public:
	void Init() override 
	{

		{
			Model lampModel = Model::Create("asset\\model\\lamp.mgm");
			GameObject* gameObject = AddGameObject();
			ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
			modelRenderer->SetModel(lampModel);
			m_Lamp = gameObject;
			m_LampModelRenderer = modelRenderer;

			for (Material material : m_LampModelRenderer->GetModel().GetData().materials) {
				auto materialData = material.GetData();
				materialData.base.a = 0;
				material.SetData(materialData);
			}
		}
		{
			GameObject* gameObject = AddGameObject();
			Texture texture = Texture::Create("asset\\texture\\particle5.png");
			Material::MATERIAL materialData{};
			materialData.baseTexture = texture;
			materialData.opacityTexture = texture;
			Material material = Material::Create(materialData);


			ParticleRenderer* particleRenderer = gameObject->AddComponent<ParticleRenderer>(
				material, 10000,
				"complied_shader\\particleUpdateCS.cso"
			);
			PARTICLE_CONSTANT constant{};
			constant.INIT_COLOR = RGBA(1.0f, 1.0f, 1.0f, 0.03f);
			constant.INIT_LIFE_MAX = 6.0f;
			constant.INIT_LIFE_MIN = 6.0f;
			constant.INIT_POSITION_RANGE = Vector4(1.0f, 1.0f, 0.0f, 0.0f);
			constant.INIT_ACCELERATION = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
			constant.INIT_SCALE = Vector4(1.0f, 1.0f, 0.0f);
			constant.INIT_SCALE_ACCELERATION = Vector4(100.0f, 100.0f, 0.0f, 0.0f);
			constant.INIT_VELOCITY = Vector4(20.0f, 1.0f, -1.0f);
			//constant.INIT_SCALE_ACCELERATION = Vector4(10.0f, 10.0f, 0.0f);
			particleRenderer->SetConstant(constant);
			particleRenderer->SetSpawnCount(2);
			gameObject->SetPosition({ 0.0f, 0.0f, 5.0f });
			flog = particleRenderer;
		}

		GetMainCamera()->GetGameObject()->SetPosition({0.0f, 0.0f, -5.0f});
	}

	void Uninit() override
	{

	}

	void Update() override
	{
		static TimeLine timeLine0(3.0f);
		timeLine0.IncreaseValue(MGUtility::GetDeltaTime());
		if (timeLine0 < 1.0f) return;

		static TimeLine flogTimeLine(3.0f);
		static TimeLine lampTimeLine(4.0f);
		static TimeLine waitTimeLine(2.0f);
		static TimeLine outTimeLine(5.0f);
		static constexpr const float CAMERA_ROTATE_SPEED = 2.0f;
		static constexpr const float CAMERA_ROTATE_X_MIN = -0.7f;
		static constexpr const float CAMERA_ROTATE_X_MAX = 0.7f;
		float deltaTime = MGUtility::GetDeltaTime();
		Camera* camera = GetMainCamera();
		Vector3 cameraRotation = camera->GetRotation();
		if (Input::GetKeyPress(VK_UP)) {
			cameraRotation.x -= CAMERA_ROTATE_SPEED * deltaTime;
			cameraRotation.x = max(cameraRotation.x, CAMERA_ROTATE_X_MIN);
		}
		if (Input::GetKeyPress(VK_DOWN)) {
			cameraRotation.x += CAMERA_ROTATE_SPEED * deltaTime;
			cameraRotation.x = min(cameraRotation.x, CAMERA_ROTATE_X_MAX);
		}
		if (Input::GetKeyPress(VK_LEFT)) {
			cameraRotation.y -= CAMERA_ROTATE_SPEED * deltaTime;
		}
		if (Input::GetKeyPress(VK_RIGHT)) {
			cameraRotation.y += CAMERA_ROTATE_SPEED * deltaTime;
		}
		camera->GetGameObject()->SetRotation(cameraRotation);

		static Vector3 startPosition{ 0.0f, 0.0f, 30.0f };
		static Vector3 endPosition{ 0.0f, 0.0f, 3.0f };
		static Vector3 fadePosition{ 0.0f, 0.0f, -10.0f };

		if (outTimeLine == 0.0f) {
			m_Lamp->SetPosition(Vector3::Lerp(startPosition, endPosition, lampTimeLine));
			for (Material material : m_LampModelRenderer->GetModel().GetData().materials) {
				auto materialData = material.GetData();
				materialData.base.a = lampTimeLine;
				material.SetData(materialData);
			}
		}
		else {
			m_Lamp->SetPosition(Vector3::Lerp(endPosition, fadePosition, outTimeLine));
			
		}
		
		if (flogTimeLine == 1.0f) {
			lampTimeLine.IncreaseValue(deltaTime);
		}
		flogTimeLine.IncreaseValue(deltaTime);
		if (lampTimeLine == 1.0f) {
			//waitTimeLine.IncreaseValue(deltaTime);
			if (Input::GetKeyTrigger(VK_SPACE)) {
				waitTimeLine.SetCurrent(waitTimeLine.GetDuration());
				flog->SetSpawnCount(0);
			}
			
		}
		if (waitTimeLine == 1.0f) {
			outTimeLine.IncreaseValue(deltaTime);
			
		}
	}
};

// DEBUG
/*{
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

	static ID3D11Buffer* debugBuffer = ([](unsigned int stride, unsigned int count) {
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.StructureByteStride = stride;
		desc.ByteWidth = stride * count;
		ID3D11Buffer* debugBuffer;
		Renderer::GetDevice()->CreateBuffer(&desc, nullptr, &debugBuffer);
		return debugBuffer;
		})(sizeof(AnimationSet::RESULT), 20000);

	Renderer::GetDeviceContext()->CopySubresourceRegion(debugBuffer, 0, 0, 0, 0, debugResultBuffer, 0, nullptr);

	D3D11_MAPPED_SUBRESOURCE mapped;
	if (Renderer::GetDeviceContext()->Map(debugBuffer, 0, D3D11_MAP_READ, 0, &mapped) == S_OK) {
		AnimationSet::RESULT data[20000];
		memcpy(data, mapped.pData, sizeof(AnimationSet::RESULT) * 20000);
		Renderer::GetDeviceContext()->Unmap(debugBuffer, 0);
	}
}*/
