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

