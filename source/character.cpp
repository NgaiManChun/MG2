#include "character.h"
#include "modelRenderer.h"
#include "MGUtility.h"

using namespace MG;

void Character::StaticInit()
{

}

void Character::StaticUninit()
{

}

void Character::Init()
{
	m_IdleState.Init(this);
}

void Character::SetModel(Model model, unsigned int lod)
{
	m_Models.push_back(model);
	ModelRenderer* modelRenderer = GetGameObject()->AddComponent<ModelRenderer>();
	modelRenderer->SetModel(model, lod);
	if (!model.GetData().animations.empty()) {
		modelRenderer->SetAnimation(0);
	}
	m_ModelRenderers.push_back(modelRenderer);

}

void Character::SetState(STATE state)
{
	m_State = state;
	if (m_State == STATE_IDLE)
	{
		m_IdleState.Init(this);
	}
	else if (m_State == STATE_RUN)
	{
		m_RunState.Init(this);
	}
	else if (state == STATE_ATTACK)
	{
		m_AttackState.Init(this);
	}
}

void Character::Update()
{

	if (m_State == STATE_IDLE)
	{
		m_IdleState.Update(this);
	}
	else if (m_State == STATE_RUN)
	{
		m_RunState.Update(this);
	}
	else if (m_State == STATE_ATTACK)
	{
		m_AttackState.Update(this);
	}

	m_MoveInput = Vector3{};
	m_AttackInput = false;
}

// アイドル状態 =====================================================
void Character::IdleState::Init(Character* character)
{
	auto& animations = character->m_Models[0].GetData().animations;
	AnimationSet animationSet = character->m_ModelRenderers[0]->GetAnimationSet();
	for (auto& modelRenderer : character->m_ModelRenderers) {
		modelRenderer->GetAnimationSet().Swap(animations[Character::IDLE_ANIMATION_SLOT], MOVE_DURATION);
	}
}

void Character::IdleState::Update(Character* character)
{
	if (character->m_AttackInput)
	{
		character->SetState(STATE_ATTACK);
	}
	else if (character->m_MoveInput.LengthSq() > 0) 
	{
		character->SetState(STATE_RUN);
	}
}


// 走る状態 =====================================================
void Character::RunState::Init(Character* character)
{
	accelerating = TimeLine(0.5f);
	isAccelerating = false;

}

void Character::RunState::Update(Character* character)
{
	if (character->m_AttackInput)
	{
		character->SetState(STATE_ATTACK);
		return;
	}

	float deltaTime = MGUtility::GetDeltaTime();
	GameObject* gameObject = character->m_GameObject;

	if (isAccelerating && !character->m_HasMoveInput)
	{
		auto& animations = character->m_Models[0].GetData().animations;
		AnimationSet animationSet = character->m_ModelRenderers[0]->GetAnimationSet();
		for (auto& modelRenderer : character->m_ModelRenderers) {
			modelRenderer->GetAnimationSet().Swap(animations[Character::IDLE_ANIMATION_SLOT], MOVE_DURATION);
		}
		isAccelerating = false;
	}
	else if (!isAccelerating && character->m_HasMoveInput) {
		auto& animations = character->m_Models[0].GetData().animations;
		AnimationSet animationSet = character->m_ModelRenderers[0]->GetAnimationSet();
		for (auto& modelRenderer : character->m_ModelRenderers) {
			modelRenderer->GetAnimationSet().Swap(animations[Character::RUN_ANIMATION_SLOT], MOVE_DURATION);
		}
		isAccelerating = true;
	}

	gameObject->SetPosition(gameObject->GetPosition() + gameObject->GetForward() * MOVE_SPEED * accelerating * deltaTime);

	if (character->m_HasMoveInput)
	{
		float rotateAngle = 0.0f;
		float maxAngle = ROTATE_SPEED * deltaTime;
		
		Vector3 forward = gameObject->GetForward();

		float dot = min(max(forward.Dot(character->m_MoveInput), -1.0f), 1.0f);
		rotateAngle = acosf(dot);

		if (rotateAngle > maxAngle) {
			// 角度が最大回転速度より大きいの場合
			rotateAngle = maxAngle;
		}

		// directは今向けようとする方向
		// directとY軸の外積でdirectにとっての右ベクトル生成して、キャラクターのforwardとの内積を求める
		if (forward.Dot(Vector3(0.0f, 1.0f, 0.0f).Cross(character->m_MoveInput)) > 0.0f) {
			// 逆時計回り
			rotateAngle = -rotateAngle;
		}
		gameObject->SetRotation(gameObject->GetRotation() + Vector3{ 0.0f, rotateAngle, 0.0f });

		// 向けようとする方向と９０度以下なら歩き始める
		if (dot > 0.0f) {
			accelerating.IncreaseValue(deltaTime);
		}
	}
	else {
		accelerating.IncreaseValue(-deltaTime);
	}

	if (accelerating == 0 && !isAccelerating) {
		character->SetState(STATE_IDLE);
	}
}


// 攻撃状態 =====================================================
void Character::AttackState::Init(Character* character)
{
	attackTime = TimeLine(1.0f);

	auto& animations = character->m_Models[0].GetData().animations;
	AnimationSet animationSet = character->m_ModelRenderers[0]->GetAnimationSet();
	for (auto& modelRenderer : character->m_ModelRenderers) {
		modelRenderer->GetAnimationSet().Swap(animations[Character::ATTACK_ANIMATION_SLOT], MOVE_DURATION);
	}
}

void Character::AttackState::Update(Character* character)
{
	if (attackTime == 1.0f) {
		if (character->m_HasMoveInput) {
			character->SetState(STATE_RUN);
		}
		else {
			character->SetState(STATE_IDLE);
		}
	}

	attackTime.IncreaseValue(MGUtility::GetDeltaTime());
}