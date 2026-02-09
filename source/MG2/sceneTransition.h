// =======================================================
// sceneTransition.h
// 
// シーン遷移処理の基底クラス
// 未着手
// =======================================================
#pragma once

namespace MG {

	class SceneTransition {
	public:
		virtual void Init();
		virtual void Uninit();
		virtual void Update();
		virtual void Draw();
	};
} // namespace MG

