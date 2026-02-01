#pragma once
#include "component.h"
#include "timeLine.h"
#include "model.h"
#include "modelAnimation.h"
#include "modelRenderer.h"
#include "vector3.h"
#include "animationFollower.h"

using namespace MG;

class Character : public Component {
	
public:
	enum STATE {
		STATE_IDLE,
		STATE_RUN,
		STATE_ATTACK
	};
	enum ANIMATION_SLOT {
		IDLE_ANIMATION_SLOT,
		RUN_ANIMATION_SLOT,
		ATTACK_ANIMATION_SLOT,
		IMPACT_ANIMATION_SLOT
	};
	static constexpr const float ROTATE_SPEED = 2.0f * XM_PI;
	static constexpr const float MOVE_SPEED = 4.0f;
	static constexpr const unsigned int MOVE_DURATION = 500;
	static constexpr const float MOVE_ACCELERATION = 8.0f;
private:
	
	static inline std::vector<ModelAnimation> s_ModelAnimations;
public:
	static void StaticInit();
	static void StaticUninit();
	//static void UpdateAll();
	BIND_STATIC_INIT(Character, Character::StaticInit)
	BIND_STATIC_UNINIT(Character, Character::StaticUninit)
	//BIND_UPDATE_ALL(Character, Character::UpdateAll)
	BIND_COMPONENT_WITHOUT_DRAW(Character)
private:
	AnimationFollower m_WeaponFollow;
	GameObject* m_Weapon;
	std::vector<Model> m_Models;
	std::vector<ModelRenderer*> m_ModelRenderers;

	STATE m_State = STATE_IDLE;

	Vector3 m_MoveInput;
	bool m_HasMoveInput;
	bool m_AttackInput;

	struct IdleState {
		void Init(Character* character);
		void Update(Character* character);
	};
	IdleState m_IdleState;

	struct RunState {
		TimeLine accelerating;
		bool isAccelerating;
		Vector3 direct;
		void Init(Character* character);
		void Update(Character* character);
	};
	RunState m_RunState;

	struct AttackState {
		TimeLine attackTime;
		void Init(Character* character);
		void Update(Character* character);
	};
	AttackState m_AttackState;

	void SetState(STATE state);


public:
	void Init() override;
	void SetModel(Model model, unsigned int = LOD_ALL);

	void Update() override;

	void MoveCommand(const Vector3& direct)
	{
		m_MoveInput = direct;
		m_MoveInput.y = 0.0f;
		m_MoveInput.Normalize();
		m_HasMoveInput = m_MoveInput.LengthSq();
	}

	void AttackCommand() {
		m_AttackInput = true;
	}

	void SetWeapon(GameObject* gameObject)
	{
		//gameObject->SetParent(GetGameObject());
		gameObject->SetPosition({ -0.02f, 0.05f, 0.0f });
		gameObject->SetRotation({ -1.04079700, 1.15359819, -1.19999921 });
		auto& namedNodeIndexes = m_Models[0].GetData().namedNodeIndexes;
		unsigned int index = namedNodeIndexes["J_Bip_R_Hand"];
		if (!static_cast<bool>(m_WeaponFollow)) {
			m_WeaponFollow = AnimationFollower::Create(gameObject->GetWorldMatrix(), m_ModelRenderers[0]->GetModelInstance(), index);
		}
		else {
			m_WeaponFollow.SetData(gameObject->GetWorldMatrix(), m_ModelRenderers[0]->GetModelInstance(), index);
		}
		m_Weapon = gameObject;
	}

	GameObject* GetWeapon() {
		return m_Weapon;
	}
};