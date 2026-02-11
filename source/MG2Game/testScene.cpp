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
public:
	void Init() override {

		m_Weapons[0] = Model::Create("asset\\model\\baseball_bat.mgm");
		m_Weapons[1] = Model::Create("asset\\model\\wooden_axe.mgm");
		m_Weapons[2] = Model::Create("asset\\model\\crowbar.mgm");


		// Player
		{
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

					int index = static_cast<int>(RANDOM_T * ARRAYSIZE(m_Weapons));
					index %= ARRAYSIZE(m_Weapons);

					modelRenderer->SetModel(m_Weapons[index], LOD_ALL);
					character->SetWeapon(gameObject);
				}
			}
		}

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
		}

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
			unsigned int enemyCount = static_cast<unsigned int>(m_EnemyTimeLines.size());
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

	}
};


