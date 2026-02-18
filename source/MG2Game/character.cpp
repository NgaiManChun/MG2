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
	
	// ”í’e”»’è
	{
		GameObject* gameObject = GetGameObject()->GetScene()->AddGameObject();
		m_Collider = gameObject->AddComponent<BoxCollider>(1);
		gameObject->SetPosition({ 0.0f, 1.0f, 0.0f });
		gameObject->SetScale({ 0.4f, 1.0f, 0.5f });
		gameObject->SetParent(GetGameObject());
	}
	
	// UŒ‚”»’è
	{
		GameObject* gameObject = GetGameObject()->GetScene()->AddGameObject();
		m_AttackCollider = gameObject->AddComponent<BoxCollider>(2);
		gameObject->SetPosition({ 0.0f, 1.0f, 0.8f });
		gameObject->SetScale({ 0.7f, 1.5f, 1.2f });
		gameObject->SetRotation({ 0.0f, 0.0f, XMConvertToRadians(-40.0f) });
		gameObject->SetParent(GetGameObject());

		m_AttackCollider->SetEnabled(false);
	}
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

bool Character::IsImpact()
{
	auto& others = m_Collider->GetOverlapColliders();
	for (Collider* other : others) {
		if ((other->GetTags() & 2) && other != m_AttackCollider) {
			m_Impact = GetGameObject()->GetWorldPosition() - other->GetGameObject()->GetParent()->GetWorldPosition();
			m_Impact.Normalize();
			m_Impact *= 5.0f;
			return true;
		}
	}
	return false;
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
	else if (state == STATE_IMPACT)
	{
		m_ImpactState.Init(this);
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
	else if (m_State == STATE_IMPACT)
	{
		m_ImpactState.Update(this);
	}

	m_MoveInput = Vector3{};
	m_AttackInput = false;
}

// ƒAƒCƒhƒ‹ó‘Ô =====================================================
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
	if (character->IsImpact())
	{
		character->SetState(STATE_IMPACT);
		return;
	}
	if (character->m_AttackInput)
	{
		character->SetState(STATE_ATTACK);
		return;
	}
	if (character->m_MoveInput.LengthSq() > 0) 
	{
		character->SetState(STATE_RUN);
	}
}


// ‘–‚éó‘Ô =====================================================
void Character::RunState::Init(Character* character)
{
	accelerating = TimeLine(0.5f);
	isAccelerating = false;

}

void Character::RunState::Update(Character* character)
{
	if (character->IsImpact()) 
	{
		character->SetState(STATE_IMPACT);
		return;
	}
	if (character->m_AttackInput)
	{
		character->SetState(STATE_ATTACK);
		return;
	}

	float deltaTime = MGUtility::GetDeltaTime();
	GameObject* gameObject = character->GetGameObject();

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
			// Šp“x‚ªÅ‘å‰ñ“]‘¬“x‚æ‚è‘å‚«‚¢‚Ìê‡
			rotateAngle = maxAngle;
		}

		// direct‚Í¡Œü‚¯‚æ‚¤‚Æ‚·‚é•ûŒü
		// direct‚ÆYŽ²‚ÌŠOÏ‚Ådirect‚É‚Æ‚Á‚Ä‚Ì‰EƒxƒNƒgƒ‹¶¬‚µ‚ÄAƒLƒƒƒ‰ƒNƒ^[‚Ìforward‚Æ‚Ì“àÏ‚ð‹‚ß‚é
		if (forward.Dot(Vector3(0.0f, 1.0f, 0.0f).Cross(character->m_MoveInput)) > 0.0f) {
			// ‹tŽžŒv‰ñ‚è
			rotateAngle = -rotateAngle;
		}
		gameObject->SetRotation(gameObject->GetRotation() + Vector3{ 0.0f, rotateAngle, 0.0f });

		// Œü‚¯‚æ‚¤‚Æ‚·‚é•ûŒü‚Æ‚X‚O“xˆÈ‰º‚È‚ç•à‚«Žn‚ß‚é
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


// UŒ‚ó‘Ô =====================================================
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
	if (character->IsImpact()) {
		character->m_AttackCollider->SetEnabled(false);
		character->SetState(STATE_IMPACT);
		return;
	}
	character->m_AttackCollider->SetEnabled(attackTime > 0.5f && attackTime < 0.8f);
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


// ”í’eó‘Ô =====================================================
void Character::ImpactState::Init(Character* character)
{
	Vector3 impact = -character->m_Impact;
	impact.y = 0;
	impact.Normalize();
	character->GetGameObject()->SetRotation({0.0f, atan2(impact.x, impact.z), 0.0f});

	impactTime = TimeLine(0.5f);

	auto& animations = character->m_Models[0].GetData().animations;
	AnimationSet animationSet = character->m_ModelRenderers[0]->GetAnimationSet();
	for (auto& modelRenderer : character->m_ModelRenderers) {
		modelRenderer->GetAnimationSet().Swap(animations[Character::IMPACT_ANIMATION_SLOT], 500);
	}
}

void Character::ImpactState::Update(Character* character)
{
	float deltaTime = MGUtility::GetDeltaTime();
	GameObject* gameObject = character->GetGameObject();
	Vector3 position = gameObject->GetPosition();
	position += character->m_Impact * deltaTime;
	gameObject->SetPosition(position);
	character->m_Impact *= 0.8f;
	if (impactTime == 1.0f) {
		if (character->m_HasMoveInput) {
			character->SetState(STATE_RUN);
		}
		else {
			character->SetState(STATE_IDLE);
		}
	}

	impactTime.IncreaseValue(deltaTime);
}