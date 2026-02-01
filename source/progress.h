// =======================================================
// progress.h
// 
// 0.0fから目的値まで累計して、
// 現在値と目的値の比率0.0f～1.0fとして使えるクラス
// 
// 作者：魏文俊（ガイ　マンチュン）　2024/07/06
// 
// 2024/12/27 「operator =」追加
// =======================================================
#pragma once
//#include <cmath>

namespace MG {
	class Progress {
	private:
		float _value;		// 現在値
		float _duration;	// 目的値、マイナス不可
		float invDivision = 1.0f;
		float percentage = 0.0f;

	public:
		// パブリック変数
		bool loop;			// 現在値が目的値超えた時ループさせるか

		// パブリック関数
		Progress() :
			Progress(1.0f, false)
		{
		}
		Progress(float duration, bool loop = false) :
			_value(0.0f),
			_duration(duration),
			loop(loop)
		{
			invDivision = 1.0f / duration;
			SetValue(0.0f);
		}

		void SetValue(float value)
		{
			_value = value;

			percentage = _value * invDivision;

			if (loop) {
				if (percentage >= 1.0f) {
					percentage -= floorf(percentage);
					_value = percentage * _duration;
				}
				else if (percentage < 0.0f) {
					percentage = abs(percentage);
					percentage -= floorf(percentage);
					percentage = 1.0f - percentage;
					_value = percentage * _duration;
				}
			}
			else {

				if (percentage > 1.0f) {
					_value = _duration;
					percentage = 1.0f;
				}
				else if (percentage < 0.0f) {
					_value = 0.0f;
					percentage = 0.0f;
				}

			}
		}

		float GetValue() const { return _value; }
		float GetDuration() const { return _duration; }

		void SetDuration(float duration)
		{
			if (_duration > 0.0f) {
				_duration = duration;
				invDivision = 1.0f / duration;
				SetValue(_value);	// はみ出し処理
			}
		}

		float IncreaseValue(float value)
		{
			SetValue(_value + value);
			return percentage;
		}

		operator float() const { return percentage; }
		void operator=(const float t) { SetValue(_duration * t); }

		

	};
} // namespace MG