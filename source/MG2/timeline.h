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
		float m_Current;
		float m_Duration;
		float m_InvDivision = 1.0f; // 割り算の逆数
		float m_Percentage = 0.0f;

	public:
		TimeLine(float duration = 1.0f, float initCurrent = 0.0f) :
			m_Duration(duration), m_Current(initCurrent)
		{
			if (m_Current < 0.0f) {
				m_Current = 0.0f;
			}
			else if (m_Current > m_Duration) {
				m_Current = m_Duration;
			}
			m_InvDivision = 1.0f / m_Duration;
			m_Percentage = m_Current * m_InvDivision;
		}

		float GetCurrent() const { return m_Current; }
		float GetDuration() const { return m_Duration; }

		float GetPercentage() const { return m_Percentage; }
		operator float() const { return m_Percentage; }

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

		// 現在値に加算する
		float IncreaseValue(float value)
		{
			return SetCurrent(m_Current + value);
		}

	};
} // namespace MG