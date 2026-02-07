#pragma once

#pragma warning(push)
#pragma warning(disable:4005)

#define _CRT_SECURE_NO_WARNINGS			// scanf のwarning防止
#include <stdio.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#define DIRECTINPUT_VERSION 0x0800		// 警告対処
#include "dinput.h"
#include "mmsystem.h"

#pragma warning(pop)

#include <DirectXMath.h>
using namespace DirectX;


//テクスチャサポートライブラリ
#include    "DirectXTex.h"
//デバッグビルドかリリースビルドでリンクするライブラリを選択する
#if _DEBUG
#pragma comment(lib, "DirectXTex_Debug.lib")    //デバッグ用
#else
#pragma comment(lib, "DirectXTex_Release.lib")  //リリース用
#endif

// =======================================================
// ライブラリのリンク
// =======================================================
#pragma	comment (lib, "d3d11.lib")
#pragma	comment (lib, "d3dcompiler.lib")
#pragma	comment (lib, "winmm.lib")
#pragma	comment (lib, "dxguid.lib")
#pragma	comment (lib, "dinput8.lib")