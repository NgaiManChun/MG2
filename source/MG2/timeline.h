// =======================================================
// timeline.h
// 
// 目標値を設定、現在値を記録し、
// 「現在値÷目標値」から0.0f～1.0fの値を求めるクラス
// =======================================================
#pragma once

namespace MG {

	class TimeLine {
	private:
		float m_Current;       // 現在値
		float m_Duration;      // 最大値（目標値）
		float m_InvDivision = 1.0f; // 割り算の逆数
		float m_Percentage = 0.0f;  // 正規化された値（0.0f～1.0f）

	public:
		TimeLine(float duration = 1.0f, float initCurrent = 0.0f) :
			m_Duration(duration), m_Current(initCurrent)
		{
			// 初期値を範囲内に収める
			if (m_Current < 0.0f) {
				m_Current = 0.0f;
			}
			else if (m_Current > m_Duration) {
				m_Current = m_Duration;
			}

			// 逆数を事前計算
			m_InvDivision = 1.0f / m_Duration;

			// 初期進行率を計算
			m_Percentage = m_Current * m_InvDivision;
		}

		// 現在値取得
		float GetCurrent() const { return m_Current; }

		// 最大値取得
		float GetDuration() const { return m_Duration; }

		// 進行率取得（0.0f～1.0f）
		float GetPercentage() const { return m_Percentage; }

		// floatとして扱うと進行率を返す
		operator float() const { return m_Percentage; }

		// =======================================================
		// 現在値を設定
		// ・範囲外は自動クランプ
		// ・進行率も同時更新
		// =======================================================
		float SetCurrent(float value)
		{
			m_Current = value;

			if (m_Current < 0.0f) {
				m_Current = 0.0f;
				m_Percentage = 0.0f;
			}
			else if (m_Current > m_Duration) {
				m_Current = m_Duration;
				m_Percentage = 1.0f;
			}
			else {
				m_Percentage = m_Current * m_InvDivision;
			}

			return m_Percentage;
		}

		// =======================================================
		// 現在値を加算
		// ・内部的にはSetCurrentを使用
		// =======================================================
		float IncreaseValue(float value)
		{
			return SetCurrent(m_Current + value);
		}

	};

} // namespace MG