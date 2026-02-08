#include "scene.h"
#include "camera.h"
#include "input.h"
#include "texture.h"
#include "model.h"
#include "animation.h"
#include "transformDivision.h"
#include "matrixDivision.h"
#include "vertexDivision.h"
#include "vertexIndexDivision.h"
#include "dynamicIndexDivision.h"
#include "modelRenderer.h"
#include "skyRenderer.h"
#include "SpriteRenderer2D.h"
#include "MGUtility.h"
#include "timeline.h"
#include "character.h"
#include "boxCollider.h"
#include "sphereCollider.h"
#include "csvResource.h"

#include "MGResource.h"

using namespace MG;

class TestScene : public Scene
{
private:
	REGISTER_SCENE(TestScene)
private:
	Model m_Model0;
	Model m_Model1;
	Model m_Model2;
	Animation m_IdleAnimation;
	Animation m_RunAnimation;
	Character* m_Player;

	BoxCollider* m_BoxColliderPlayer;
	BoxCollider* m_BoxColliderTarget;
	SphereCollider* m_SphereColliderTarget;

	Model m_Weapons[3];

	std::vector<Character*> m_Enemies;
	std::vector<TimeLine> m_EnemyTimeLines;
	std::vector<Vector3> m_EnemyDirections;
public:
	void Init() override {

		MGResource resource("config.pak");
		int x = 0;

		/*MGResource resource;
		resource.Add("inputLayout\\general.csv");
		resource.Add("inputLayout\\test.csv");
		resource.Write("inputLayout\\resource.pak");
		resource.Release();

		resource = MGResource("inputLayout\\resource.pak");
		auto file = resource.GetFile("inputLayout\\general.csv");
		CSVResource csv(file.data, file.size);

		resource.Release();*/
		/*Matrix4x4 data[3] = { Matrix4x4{}, Matrix4x4{}, Matrix4x4{} };
		MatrixDivision d0 = MatrixDivision::Create(3, data);
		for (int i = 0; i < 3; i++) { data[i] *= 10; }
		MatrixDivision d1 = MatrixDivision::Create(3, data);
		for (int i = 0; i < 3; i++) { data[i] *= 10; }
		MatrixDivision d2 = MatrixDivision::Create(3, data);

		d1.Release();
		d0.Release();

		MatrixDivision::Pad();

		d2.Release();

		for (int i = 0; i < 3; i++) { data[i] *= 10; }
		MatrixDivision d3 = MatrixDivision::Create(3, data);

		MatrixDivision::Pad();

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
			})(sizeof(Matrix4x4), 1000);

		Renderer::GetDeviceContext()->CopySubresourceRegion(debugBuffer, 0, 0, 0, 0, MatrixDivision::GetDataBuffer(), 0, nullptr);

		D3D11_MAPPED_SUBRESOURCE mapped;
		if (Renderer::GetDeviceContext()->Map(debugBuffer, 0, D3D11_MAP_READ, 0, &mapped) == S_OK) {
			Matrix4x4 data[1000];
			memcpy(data, mapped.pData, sizeof(Matrix4x4) * 1000);
			Renderer::GetDeviceContext()->Unmap(debugBuffer, 0);
		}*/


		m_Weapons[0] = Model::Create("asset\\model\\baseball_bat.mgm");
		m_Weapons[1] = Model::Create("asset\\model\\wooden_axe.mgm");
		m_Weapons[2] = Model::Create("asset\\model\\crowbar.mgm");



		/*RGBA colors[3] = {
			{1.0f, 0.5f, 0.5f, 1.0f},
			{1.0f, 1.0f, 0.5f, 1.0f},
			{1.0f, 0.5f, 1.0f, 1.0f}
		};

		for (int i = 0; i < ARRAYSIZE(weapons); i++) {
			auto materialData = weapons[i].GetData().materials[0].GetData();
			materialData.base = colors[i];
			weapons[i].GetData().materials[0].SetData(materialData);
		}*/

		// Player
		{
			//alice
			Model model0 = Model::Create("asset\\model\\player.mgm");
			Model model1 = Model::Create("asset\\model\\player_lod1.mgm");
			Model model2 = Model::Create("asset\\model\\player_lod2.mgm");


			Animation idleAnimation = Animation::Create("asset\\animation\\player_idle.mga");
			Animation runAnimation = Animation::Create("asset\\animation\\player_run.mga");
			Animation attackAnimation = Animation::Create("asset\\animation\\player_attack.mga");
			Animation impactAnimation = Animation::Create("asset\\animation\\player_impact.mga");
			//alice_impact
			model0.BindAnimation(Character::IDLE_ANIMATION_SLOT, idleAnimation, true);
			model0.BindAnimation(Character::RUN_ANIMATION_SLOT, runAnimation, true);
			model0.BindAnimation(Character::ATTACK_ANIMATION_SLOT, attackAnimation, false);
			model0.BindAnimation(Character::IMPACT_ANIMATION_SLOT, impactAnimation, false);
			//IMPACT_ANIMATION_SLOT

			{
				GameObject* gameObject = AddGameObject();
				Character* character = gameObject->AddComponent<Character>();
				character->SetModel(model0, LOD_0);
				character->SetModel(model1, LOD_1);
				character->SetModel(model2, LOD_2 | LOD_3 | LOD_4);
				m_Player = character;



				// BoxCollider
				{
					/*Model model = Model::Create("asset\\model\\box.mgm");
					GameObject* gameObject = AddGameObject();

					ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
					modelRenderer->SetModel(model, LOD_ALL);

					m_BoxColliderPlayer = gameObject->AddComponent<BoxCollider>();

					gameObject->SetPosition({ 0.0f, 1.0f, 0.8f });
					gameObject->SetScale({ 0.7f, 1.5f, 1.2f });
					gameObject->SetRotation({ 0.0f, 0.0f, XMConvertToRadians(-40.0f) });
					gameObject->SetParent(m_Player->GetGameObject());*/
				}

			}

			// •Ší
			{
				GameObject* gameObject = AddGameObject();
				ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
				modelRenderer->SetModel(m_Weapons[0], LOD_ALL);
				m_Player->SetWeapon(gameObject);
			}




		}

		// Enemy
		{
			const unsigned int ENEMY_COUNT = 1000;
			const float RANGE = 30.0f;
			m_Enemies.reserve(ENEMY_COUNT);
			m_EnemyTimeLines.reserve(ENEMY_COUNT);
			m_EnemyDirections.resize(ENEMY_COUNT);
			Model model0 = Model::Create("asset\\model\\alice.mgm");
			Model model1 = Model::Create("asset\\model\\alice_lod1.mgm");
			Model model2 = Model::Create("asset\\model\\alice_lod2.mgm");
			Animation idleAnimation = Animation::Create("asset\\animation\\alice_idle.mga");
			Animation runAnimation = Animation::Create("asset\\animation\\alice_run.mga");
			Animation attackAnimation = Animation::Create("asset\\animation\\alice_attack.mga");
			Animation impactAnimation = Animation::Create("asset\\animation\\alice_impact.mga");
			model0.BindAnimation(Character::IDLE_ANIMATION_SLOT, idleAnimation, true);
			model0.BindAnimation(Character::RUN_ANIMATION_SLOT, runAnimation, true);
			model0.BindAnimation(Character::ATTACK_ANIMATION_SLOT, attackAnimation, false);
			model0.BindAnimation(Character::IMPACT_ANIMATION_SLOT, impactAnimation, false);
			for (int i = 0; i < ENEMY_COUNT; i++)
			{
				GameObject* gameObject = AddGameObject();
				Character* character = gameObject->AddComponent<Character>();
				character->SetModel(model0, LOD_0);
				character->SetModel(model1, LOD_1 | LOD_2);
				character->SetModel(model2, LOD_3 | LOD_4);
				gameObject->SetPosition({ RANDOM_T * RANDOM_SIGN * RANGE, 0.0f, RANDOM_T * RANDOM_SIGN * RANGE });
				m_Enemies.push_back(character);
				m_EnemyTimeLines.push_back(TimeLine(RANDOM_T * 5.0f));

				// •Ší
				{
					GameObject* gameObject = AddGameObject();
					ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();

					int index = RANDOM_T * ARRAYSIZE(m_Weapons);
					index %= ARRAYSIZE(m_Weapons);

					modelRenderer->SetModel(m_Weapons[index], LOD_ALL);
					character->SetWeapon(gameObject);
				}

				// BoxCollider
				{
					//Model model = Model::Create("asset\\model\\box.mgm");
					//GameObject* gameObject = AddGameObject();

					////ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
					////modelRenderer->SetModel(model, LOD_ALL);

					//gameObject->AddComponent<BoxCollider>();
					//gameObject->SetPosition({ 0.0f, 1.0f, 0.0f });
					//gameObject->SetParent(character->GetGameObject());
				}
			}
		}

		static Texture white = Texture::Create("asset\\texture\\white.png");
		/*Material material = Material::Create();
		auto materialData = material.GetData();
		materialData.base = RGBA{ 1.0f, 1.0f, 1.0f, 1.0f };
		materialData.baseTexture = Texture::Create("asset\\texture\\grass.jpg");
		materialData.opacityTexture = white;
		material.SetData(materialData);*/

		// Sky
		{
			Model model = Model::Create("asset\\model\\sky.mgm");
			GameObject* gameObject = AddGameObject();
			SkyRenderer* modelRenderer = gameObject->AddComponent<SkyRenderer>();
			modelRenderer->SetModel(model);
			gameObject->SetScale({ 100.0f, 100.0f, 100.0f });
		}


		// Plane
		{
			Model model = Model::Create("asset\\model\\ground.mgm");

			for (int x = -1; x < 2; x++) {
				for (int z = -1; z < 2; z++) {
					GameObject* gameObject = AddGameObject();
					ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
					modelRenderer->SetModel(model, LOD_ALL);
					gameObject->SetPosition({ 100.0f * x, 0.0f, 100.0f * z });
				}
			}
			/*GameObject* gameObject = AddGameObject();
			ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
			modelRenderer->SetModel(model, LOD_ALL);*/

			//model.GetData().materials[0] = material;

			//modelRenderer->SetMaterial(material, 0);
			//gameObject->SetScale({ 500.0f, 1.0f, 500.0f });
		}

		// BoxCollider
		{
			/*Model model = Model::Create("asset\\model\\box.mgm");
			GameObject* gameObject = AddGameObject();

			ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
			modelRenderer->SetModel(model, LOD_ALL);

			m_BoxColliderTarget = gameObject->AddComponent<BoxCollider>();

			gameObject->SetPosition({ 5.0f, 1.0f, 0.0f });
			gameObject->SetRotation({ 0.5f, 0.5f, 0.5f });
			gameObject->SetScale({ 1.0f, 2.0f, 3.0f });*/
		}

		// SphereCollider
		{
			Model model = Model::Create("asset\\model\\sphere.mgm");
			GameObject* gameObject = AddGameObject();

			//ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
			//modelRenderer->SetModel(model, LOD_ALL);

			m_SphereColliderTarget = gameObject->AddComponent<SphereCollider>();

			gameObject->SetPosition({ 4.0f, 1.0f, 0.0f });
			//gameObject->SetRotation({ 0.5f, 0.5f, 0.5f });
			gameObject->SetScale({ 1.0f, 2.0f, 5.0f });
		}

		// SpriteRenderer2D
		{
			/*GameObject* gameObject = AddGameObject();
			SpriteRenderer2D* spriteRenderer2D = gameObject->AddComponent<SpriteRenderer2D>();

			Material material = Material::Create();
			auto materialData = material.GetData();
			materialData.base = RGBA{ 1.0f, 1.0f, 1.0f, 1.0f };
			materialData.baseTexture = Texture::Create("asset\\texture\\grass.jpg");
			materialData.opacityTexture = white;
			material.SetData(materialData);
			spriteRenderer2D->SetMaterial(material);

			float width = MGUtility::GetScreenWidth() * 0.3f;
			float height = MGUtility::GetScreenHeight() * 0.3f;
			Vector3 position = { (width - MGUtility::GetScreenWidth()) * 0.5f, (height - MGUtility::GetScreenHeight()) * 0.5f };

			gameObject->SetScale({ width, height, 0.0f });
			gameObject->SetPosition(position);*/
		}

		GetMainCamera()->GetGameObject()->SetPosition({ 0.0f, 1.0f, -5.0f });
		//GetMainCamera()->GetGameObject()->SetRotation({ 0.3f, 0.0f, 0.0f });

		/*DynamicIndexDivision vertexIndexDivisions[3];
		for (int i = 0; i < 3; i++) {
			std::vector<UINT> data(5);
			for (int k = 0; k < data.size(); k++) {
				data[k] = 100 + i * 10 + k;
			}
			vertexIndexDivisions[i] = DynamicIndexDivision::Create(data.size(), data.data());
		}

		vertexIndexDivisions[0].Release();
		vertexIndexDivisions[1].Release();

		for (int i = 0; i < 3; i++) {
			std::vector<UINT> data(5);
			for (int k = 0; k < data.size(); k++) {
				data[k] = 200 + i * 10 + k;
			}
			vertexIndexDivisions[i] = DynamicIndexDivision::Create(data.size(), data.data());
		}*/


		//vertexIndexDivisions[0].Release();
		//DynamicIndexDivision::Pad();

	}

	void Uninit() override
	{

	}

	void Update() override
	{
		float deltaTime = MGUtility::GetDeltaTime();

		// PlayerController
		{
			static constexpr const float CAMERA_ROTATE_SPEED = 2.0f;
			static constexpr const float CAMERA_ROTATE_X_MIN = -0.7f;
			static constexpr const float CAMERA_ROTATE_X_MAX = 0.7f;
			static constexpr const float CAMERA_DISTANCE = 2.0f;
			static const Vector3 CAMERA_OFFSET = { 0.0f, 1.2f, 0.0f };

			Camera* camera = GetMainCamera();


			//m_Weapons
			if (Input::GetKeyPress('1')) {
				ModelRenderer* modelRenderer = m_Player->GetWeapon()->GetComponent<ModelRenderer>();
				if (modelRenderer) {
					modelRenderer->SetModel(m_Weapons[0]);
				}
			}
			else if (Input::GetKeyPress('2')) {
				ModelRenderer* modelRenderer = m_Player->GetWeapon()->GetComponent<ModelRenderer>();
				if (modelRenderer) {
					modelRenderer->SetModel(m_Weapons[1]);
				}
			}
			else if (Input::GetKeyPress('3')) {
				ModelRenderer* modelRenderer = m_Player->GetWeapon()->GetComponent<ModelRenderer>();
				if (modelRenderer) {
					modelRenderer->SetModel(m_Weapons[2]);
				}
			}


			// Move
			bool move = false;
			Vector3 direct{ 0.0f, 0.0f, 0.0f };
			if (Input::GetKeyPress('W')) {
				direct += camera->GetForward();
			}
			if (Input::GetKeyPress('S')) {
				direct -= camera->GetForward();
			}
			if (Input::GetKeyPress('A')) {
				direct -= camera->GetRight();
			}
			if (Input::GetKeyPress('D')) {
				direct += camera->GetRight();
			}
			direct.y = 0.0f;
			m_Player->MoveCommand(direct);

			// Attack
			if (Input::GetKeyTrigger('J')) {
				m_Player->AttackCommand();
			}

			// Camera
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
			Vector3 targetPoint = m_Player->GetGameObject()->GetPosition() + CAMERA_OFFSET;
			Vector3 cameraMove = (targetPoint - camera->GetForward() * CAMERA_DISTANCE) - camera->GetPosition();
			cameraMove *= 7.0f * deltaTime;
			camera->GetGameObject()->SetPosition(
				camera->GetPosition() + cameraMove
			);
		}

		// EnemyController
		{
			unsigned int enemyCount = m_EnemyTimeLines.size();
			for (unsigned int i = 0; i < enemyCount; i++) {
				TimeLine& timeLine = m_EnemyTimeLines[i];
				if (timeLine == 1.0f) {
					m_Enemies[i]->AttackCommand();
					timeLine.SetCurrent(0.0f);
					if (RANDOM_SIGN > 0) {
						m_EnemyDirections[i] = Vector3::GetSignedRandom();
						m_EnemyDirections[i].y = 0.0f;
					}
					else {
						m_EnemyDirections[i] = Vector3{};
					}

				}
				else {
					m_Enemies[i]->MoveCommand(m_EnemyDirections[i]);
					timeLine.IncreaseValue(deltaTime);
				}

			}
		}

		/*if (m_BoxColliderTarget->HasOverlap()) {
			static Model model = Model::Create("asset\\model\\box.mgm");
			auto& material = model.GetData().materials[0];
			auto materialData = material.GetData();
			materialData.base = RGBA(1.0f, 0.0f, 0.0f, 1.0f);
			material.SetData(materialData);
		}
		else {
			static Model model = Model::Create("asset\\model\\box.mgm");
			auto& material = model.GetData().materials[0];
			auto materialData = material.GetData();
			materialData.base = RGBA(1.0f, 1.0f, 1.0f, 1.0f);
			material.SetData(materialData);
		}*/

		//if (m_BoxColliderPlayer->IsEnabled()) {
		//	if (m_BoxColliderTarget->Overlap(m_BoxColliderPlayer)) {
		//		static Model model = Model::Create("asset\\model\\box.mgm");
		//		auto& material = model.GetData().materials[0];
		//		auto materialData = material.GetData();
		//		materialData.base = RGBA(1.0f, 0.0f, 0.0f, 1.0f);
		//		material.SetData(materialData);
		//	}
		//	else {
		//		static Model model = Model::Create("asset\\model\\box.mgm");
		//		auto& material = model.GetData().materials[0];
		//		auto materialData = material.GetData();
		//		materialData.base = RGBA(1.0f, 1.0f, 1.0f, 1.0f);
		//		material.SetData(materialData);
		//	}

		//	if (m_SphereColliderTarget->Overlap(m_BoxColliderPlayer)) {
		//		static Model model = Model::Create("asset\\model\\sphere.mgm");
		//		auto& material = model.GetData().materials[0];
		//		auto materialData = material.GetData();
		//		materialData.base = RGBA(1.0f, 0.0f, 0.0f, 1.0f);
		//		material.SetData(materialData);
		//	}
		//	else {
		//		static Model model = Model::Create("asset\\model\\sphere.mgm");
		//		auto& material = model.GetData().materials[0];
		//		auto materialData = material.GetData();
		//		materialData.base = RGBA(1.0f, 1.0f, 1.0f, 1.0f);
		//		material.SetData(materialData);
		//	}

		//	if (Input::GetKeyPress('P')) {
		//		m_BoxColliderPlayer->GetGameObject()->Destroy();
		//	}
		//}
		//else {
		//	if (Input::GetKeyPress('O')) {
		//		Model model = Model::Create("asset\\model\\box.mgm");
		//		GameObject* gameObject = AddGameObject();

		//		ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
		//		modelRenderer->SetModel(model, LOD_ALL);

		//		m_BoxColliderPlayer = gameObject->AddComponent<BoxCollider>();

		//		gameObject->SetPosition({ 0.0f, 1.0f, 0.0f });
		//		//gameObject->SetScale({ 2.0f, 1.0f, 1.0f });
		//		gameObject->SetParent(m_Player->GetGameObject());
		//	}
		//	
		//}





	}
};


