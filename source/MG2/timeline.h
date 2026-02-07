#pragma once

namespace MG {
	class TimeLine {
	private:
		float m_Current;
		float m_Duration;
		float m_InvDivision = 1.0f;
		float m_Percentage = 0.0f;
	public:

		// パブリック関数
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

		float SetCurrent(float value)
		{
			m_Current = value;
			if (m_Current < 0.0f) {
				m_Current = 0.0f;
			}
			else if (m_Current > m_Duration) {
				m_Current = m_Duration;
			}
			m_Percentage = m_Current * m_InvDivision;
			return m_Percentage;
		}

		float IncreaseValue(float value)
		{
			return SetCurrent(m_Current + value);
		}

		operator float() const { return m_Percentage; }

	};
} // namespace MG