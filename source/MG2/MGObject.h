// =======================================================
// MGObject.h
// 
// ファイル出力用構造体
// 
// 作者：魏文俊（ガイ　マンチュン）　2024/11/09
// =======================================================
#pragma once

namespace MG {

	enum MGOBJECT_TYPE {
		MGOBJECT_TYPE_MODEL,
		MGOBJECT_TYPE_ANIMATION,
		MGOBJECT_TYPE_ARRANGEMENT,
		MGOBJECT_TYPE_DATA
	};

	class MGObject {
	public:
		MGOBJECT_TYPE type;
		size_t size;
		char* data;
		void Release();

		operator bool() const
		{
			return data;
		}
	};

	MGObject LoadMGO(const char* fileName);

} // namespace MG