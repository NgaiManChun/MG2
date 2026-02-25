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
#include "particleRenderer.h"
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

	unsigned int m_EnemyNum = 1000;
	Vector3 m_EnemyRange{ 50.0f, 0.0f, 50.0f };


public:
	void Init() override {

		Model palyer_lod0;
		Model palyer_lod1;
		Model palyer_lod2;
		Model enemy_lod0;
		Model enemy_lod1;
		Model enemy_lod2;
		Model weapon0;
		Model weapon1;
		Model weapon2;
		Model sky;
		Model ground;
		Animation playerIdleAnimation;
		Animation playerRunAnimation;
		Animation playerAttackAnimation;
		Animation playerImpactAnimation;
		Animation enemyIdleAnimation;
		Animation enemyRunAnimation;
		Animation enemyAttackAnimation;
		Animation enemyImpactAnimation;

		// コンフィグ読み込み
		{
			MGResource configResource = MGResource("config.pak");

			// TestScene設定
			auto textConfig = configResource.GetFile("test.csv");
			if (textConfig) {
				auto csv = CSVResource(textConfig.data, textConfig.size);
				m_EnemyNum = csv["ENEMY_NUM"]["value"];
				m_EnemyRange = csv["ENEMY_RANGE"]["value"];
			}

			// アセット設定
			auto modelConfig = configResource.GetFile("asset.csv");
			if (modelConfig) {
				auto csv = CSVResource(modelConfig.data, modelConfig.size);
				palyer_lod0 = Model::Create(csv["PLAYER_LOD0"]["value"]);
				palyer_lod1 = Model::Create(csv["PLAYER_LOD1"]["value"]);
				palyer_lod2 = Model::Create(csv["PLAYER_LOD2"]["value"]);
				enemy_lod0 = Model::Create(csv["ENEMY_LOD0"]["value"]);
				enemy_lod1 = Model::Create(csv["ENEMY_LOD1"]["value"]);
				enemy_lod2 = Model::Create(csv["ENEMY_LOD2"]["value"]);
				m_Weapons[0] = Model::Create(csv["WEAPON_0"]["value"]);
				m_Weapons[1] = Model::Create(csv["WEAPON_1"]["value"]);
				m_Weapons[2] = Model::Create(csv["WEAPON_2"]["value"]);
				sky = Model::Create(csv["SKY"]["value"]);
				ground = Model::Create(csv["GROUND"]["value"]);
				playerIdleAnimation = Animation::Create(csv["PLAYER_IDLE"]["value"]);
				playerRunAnimation = Animation::Create(csv["PLAYER_RUN"]["value"]);
				playerAttackAnimation = Animation::Create(csv["PLAYER_ATTACK"]["value"]);
				playerImpactAnimation = Animation::Create(csv["PLAYER_IMPACT"]["value"]);
				palyer_lod0.BindAnimation(Character::IDLE_ANIMATION_SLOT, playerIdleAnimation, true);
				palyer_lod0.BindAnimation(Character::RUN_ANIMATION_SLOT, playerRunAnimation, true);
				palyer_lod0.BindAnimation(Character::ATTACK_ANIMATION_SLOT, playerAttackAnimation, false);
				palyer_lod0.BindAnimation(Character::IMPACT_ANIMATION_SLOT, playerImpactAnimation, false);
				enemyIdleAnimation = Animation::Create(csv["ENEMY_IDLE"]["value"]);
				enemyRunAnimation = Animation::Create(csv["ENEMY_RUN"]["value"]);
				enemyAttackAnimation = Animation::Create(csv["ENEMY_ATTACK"]["value"]);
				enemyImpactAnimation = Animation::Create(csv["ENEMY_IMPACT"]["value"]);
				enemy_lod0.BindAnimation(Character::IDLE_ANIMATION_SLOT, enemyIdleAnimation, true);
				enemy_lod0.BindAnimation(Character::RUN_ANIMATION_SLOT, enemyRunAnimation, true);
				enemy_lod0.BindAnimation(Character::ATTACK_ANIMATION_SLOT, enemyAttackAnimation, false);
				enemy_lod0.BindAnimation(Character::IMPACT_ANIMATION_SLOT, enemyImpactAnimation, false);
			}

			configResource.Release();
		}

		// プレイヤー設定
		{
			GameObject* gameObject = AddGameObject();
			Character* character = gameObject->AddComponent<Character>();
			character->SetModel(palyer_lod0, LOD_0);
			character->SetModel(palyer_lod1, LOD_1);
			character->SetModel(palyer_lod2, LOD_2 | LOD_3 | LOD_4);
			m_Player = character;

			// プレイヤー武器設定
			{
				GameObject* gameObject = AddGameObject();
				ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
				modelRenderer->SetModel(m_Weapons[0], LOD_ALL);
				m_Player->SetWeapon(gameObject);
			}
		}

		// エネミー設定
		{
			m_Enemies.reserve(m_EnemyNum);
			m_EnemyTimeLines.reserve(m_EnemyNum);
			m_EnemyDirections.resize(m_EnemyNum);
			for (int i = 0; i < m_EnemyNum; i++)
			{
				GameObject* gameObject = AddGameObject();
				Character* character = gameObject->AddComponent<Character>();
				character->SetModel(enemy_lod0, LOD_0);
				character->SetModel(enemy_lod1, LOD_1 | LOD_2);
				character->SetModel(enemy_lod2, LOD_3 | LOD_4);
				gameObject->SetPosition(m_EnemyRange * Vector3{ RANDOM_T * RANDOM_SIGN, 0.0f, RANDOM_T * RANDOM_SIGN });
				m_Enemies.push_back(character);
				m_EnemyTimeLines.push_back(TimeLine(RANDOM_T * 5.0f));

				// エネミー武器設定
				{
					GameObject* gameObject = AddGameObject();
					ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();

					// ランタイム武器
					int index = static_cast<int>(RANDOM_T * ARRAYSIZE(m_Weapons));
					index %= ARRAYSIZE(m_Weapons);

					modelRenderer->SetModel(m_Weapons[index], LOD_ALL);
					character->SetWeapon(gameObject);
				}
			}
		}

		// スカイスフィア設定
		{
			GameObject* gameObject = AddGameObject();
			SkyRenderer* modelRenderer = gameObject->AddComponent<SkyRenderer>();
			modelRenderer->SetModel(sky);
			gameObject->SetScale({ 100.0f, 100.0f, 100.0f });
		}

		// 地面設定
		{
			for (int x = -1; x < 2; x++) {
				for (int z = -1; z < 2; z++) {
					GameObject* gameObject = AddGameObject();
					ModelRenderer* modelRenderer = gameObject->AddComponent<ModelRenderer>();
					modelRenderer->SetModel(ground, LOD_ALL);
					gameObject->SetPosition({ 100.0f * x, 0.0f, 100.0f * z });
				}
			}
		}

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

			// 移動
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

			// 攻撃
			if (Input::GetKeyTrigger('J')) {
				m_Player->AttackCommand();
			}

			// カメラ
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
			unsigned int enemyCount = static_cast<unsigned int>(m_EnemyTimeLines.size());
			for (unsigned int i = 0; i < enemyCount; i++) {
				TimeLine& timeLine = m_EnemyTimeLines[i];
				if (timeLine == 1.0f) {
					m_Enemies[i]->AttackCommand();
					timeLine.SetCurrent(0.0f);

					const Vector3& currentPosition = m_Enemies[i]->GetGameObject()->GetPosition();
					if (currentPosition.LengthSq() > m_EnemyRange.LengthSq()) {
						// 圏外に出た
						m_EnemyDirections[i] = -currentPosition;
					}
					else {
						if (RANDOM_SIGN > 0) {
							m_EnemyDirections[i] = Vector3::GetSignedRandom();
							m_EnemyDirections[i].y = 0.0f;
						}
						else {
							m_EnemyDirections[i] = Vector3{};
						}
					}
				}
				else {
					m_Enemies[i]->MoveCommand(m_EnemyDirections[i]);
					timeLine.IncreaseValue(deltaTime);
				}

			}
		}

	}
};


