#pragma once
#include <Windows.h>
#include <vector>

class Input
{
public:

	enum INPUT_TYPE
	{
		INPUT_TYPE_KEY,
		INPUT_TYPE_PAD
	};

	enum PAD_STATE_CODE{
		PAD_UP,
		PAD_DOWN,
		PAD_LEFT,
		PAD_RIGHT,
		PAD_START,
		PAD_BACK,
		PAD_L3,
		PAD_R3,
		PAD_LB,
		PAD_RB,
		PAD_A,
		PAD_B,
		PAD_X,
		PAD_Y,
		
		PAD_LEFT_ANALOG_X,
		PAD_LEFT_ANALOG_Y,
		PAD_RIGHT_ANALOG_X,
		PAD_RIGHT_ANALOG_Y,
		
		
		PAD_LT,
		PAD_RT,
		
		PAD_STATE_CODE_MAX
	};

	struct INPUT_LAYOUT
	{
		BYTE slot;
		INPUT_TYPE inputType;
		BYTE rawState;
		float value = 1.0f;
	};

	struct INPUT_VIEW
	{
		std::vector<INPUT_LAYOUT> keyInputLayouts;
		std::vector<INPUT_LAYOUT> padInputLayouts;
	};

private:
	static inline BYTE s_OldKeyState[256];
	static inline BYTE s_KeyState[256];
	static inline float s_OldPadState[PAD_STATE_CODE_MAX];
	static inline float s_PadState[PAD_STATE_CODE_MAX];

	static constexpr unsigned int INPUT_VIEW_NUM = 2;
	static constexpr unsigned int INPUT_VIEW_STATE_NUM = 256;

	static inline INPUT_VIEW s_InputViews[INPUT_VIEW_NUM];
	static float inline s_OldState[INPUT_VIEW_STATE_NUM * INPUT_VIEW_NUM];
	static float inline s_State[INPUT_VIEW_STATE_NUM * INPUT_VIEW_NUM];
public:
	static void Init();
	static void Uninit();
	static void Update();

	static bool IsKeyPress( BYTE keyCode );
	static bool IsKeyTrigger( BYTE keyCode );
	static bool IsPadPress(BYTE inputCode);
	static bool IsPadTrigger(BYTE inputCode);

	static void SetInputLayouts(INPUT_LAYOUT* inputLayouts, size_t size, unsigned int inputViewIndex = 0);

	static bool IsPress(BYTE slot, unsigned int inputViewIndex = 0);
	static bool IsTrigger(BYTE slot, unsigned int inputViewIndex = 0);
	static float GetValue(BYTE slot, unsigned int inputViewIndex = 0);

};