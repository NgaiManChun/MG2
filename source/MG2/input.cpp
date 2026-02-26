#include "input.h"

#include <xinput.h>
#pragma comment (lib, "xinput.lib")

void Input::Init()
{

	memset( s_OldKeyState, 0, 256 );
	memset( s_KeyState, 0, 256 );

    memset(s_OldPadState, 0, sizeof(float) * PAD_STATE_CODE_MAX);
    memset(s_PadState, 0, sizeof(float) * PAD_STATE_CODE_MAX);

	memset(s_State, 0, sizeof(float) * INPUT_VIEW_STATE_NUM * INPUT_VIEW_NUM);
	memset(s_OldState, 0, sizeof(float) * INPUT_VIEW_STATE_NUM * INPUT_VIEW_NUM);
}

void Input::Uninit()
{


}

void Input::Update()
{
    // キーボードステータス
	memcpy( s_OldKeyState, s_KeyState, 256 );
	GetKeyboardState( s_KeyState );

    // XInputコントローラー
    {
        memcpy(s_OldPadState, s_PadState, sizeof(float) * PAD_STATE_CODE_MAX);
        memset(s_PadState, 0, sizeof(float) * PAD_STATE_CODE_MAX);

        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        ZeroMemory(&s_PadState, sizeof(float) * PAD_STATE_CODE_MAX);
        DWORD xInputResult = XInputGetState(0, &state);

        if (xInputResult == ERROR_SUCCESS) {
            s_PadState[PAD_UP] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
            s_PadState[PAD_DOWN] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) >> 1;
            s_PadState[PAD_LEFT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) >> 2;
            s_PadState[PAD_RIGHT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) >> 3;
            s_PadState[PAD_START] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) >> 4;
            s_PadState[PAD_BACK] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) >> 5;
            s_PadState[PAD_L3] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) >> 6;
            s_PadState[PAD_R3] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) >> 7;
            s_PadState[PAD_LB] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) >> 8;
            s_PadState[PAD_RB] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) >> 9;
            
            s_PadState[PAD_A] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) >> 12;
            s_PadState[PAD_B] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) >> 13;
            s_PadState[PAD_X] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) >> 14;
            s_PadState[PAD_Y] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) >> 15;

            if (abs(state.Gamepad.sThumbLX) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            {
                s_PadState[PAD_LEFT_ANALOG_X] = (float)state.Gamepad.sThumbLX / 32767.0f;
            }
            if (abs(state.Gamepad.sThumbLY) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            {
                s_PadState[PAD_LEFT_ANALOG_Y] = (float)state.Gamepad.sThumbLY / 32767.0f;
            }
            if (abs(state.Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
            {
                s_PadState[PAD_RIGHT_ANALOG_X] = (float)state.Gamepad.sThumbRX / 32767.0f;
            }
            if (abs(state.Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
            {
                s_PadState[PAD_RIGHT_ANALOG_Y] = (float)state.Gamepad.sThumbRY / 32767.0f;
            }
        }
    }

    // バーチャルステータス
    memcpy(s_OldState, s_State, sizeof(float) * INPUT_VIEW_STATE_NUM * INPUT_VIEW_NUM);
    memset(s_State, 0, sizeof(float) * INPUT_VIEW_STATE_NUM * INPUT_VIEW_NUM);
	for (unsigned int i = 0; i < INPUT_VIEW_NUM; i++) {
		INPUT_VIEW& inputView = s_InputViews[i];
		unsigned int stateOffset = INPUT_VIEW_STATE_NUM * i;

		for (auto& inputLayout : inputView.keyInputLayouts) {
			if ((s_KeyState[inputLayout.rawState] & 0x80)) {
				s_State[stateOffset + inputLayout.slot] += inputLayout.value;
			}
		}

        for (auto& inputLayout : inputView.padInputLayouts) {
            s_State[stateOffset + inputLayout.slot] += s_PadState[inputLayout.rawState];
        }
	}

}

bool Input::IsKeyPress(BYTE keyCode)
{
	return (s_KeyState[keyCode] & 0x80);
}

bool Input::IsKeyTrigger(BYTE keyCode)
{
	return ((s_KeyState[keyCode] & 0x80) && !(s_OldKeyState[keyCode] & 0x80));
}

bool Input::IsPadPress(BYTE inputCode)
{
    return s_PadState[inputCode];
}

bool Input::IsPadTrigger(BYTE inputCode)
{
    return s_PadState[inputCode] && !(s_OldPadState[inputCode]);
}

void Input::SetInputLayouts(INPUT_LAYOUT* inputLayouts, size_t size, unsigned int inputViewIndex)
{
	if (inputViewIndex >= 8 || inputViewIndex < 0)
		return;

	INPUT_VIEW& inputView = s_InputViews[inputViewIndex];
	inputView.keyInputLayouts.clear();

	for (size_t i = 0; i < size; i++) {
		if (inputLayouts[i].inputType == INPUT_TYPE_KEY) {
			inputView.keyInputLayouts.push_back(inputLayouts[i]);
		}
        else if (inputLayouts[i].inputType == INPUT_TYPE_PAD) {
            inputView.padInputLayouts.push_back(inputLayouts[i]);
        }
	}
}

bool Input::IsPress(BYTE slot, unsigned int inputViewIndex)
{
	return s_State[INPUT_VIEW_STATE_NUM * inputViewIndex + slot];
}

bool Input::IsTrigger(BYTE slot, unsigned int inputViewIndex)
{
	return s_State[INPUT_VIEW_STATE_NUM * inputViewIndex + slot] && !s_OldState[INPUT_VIEW_STATE_NUM * inputViewIndex + slot];
}

float Input::GetValue(BYTE slot, unsigned int inputViewIndex)
{
	return s_State[INPUT_VIEW_STATE_NUM * inputViewIndex + slot];
}
