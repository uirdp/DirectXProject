#include "Timer.h"

Timer::Timer()
{
	QueryPerformanceFrequency(&m_Frequency);
	Reset();
}

void Timer::Reset()
{
	QueryPerformanceCounter(&m_StartTime);
	m_ElapsedTime = 0.0;
}

double Timer::GetElapsedTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	// microseconds
	m_ElapsedTime = static_cast<double>(currentTime.QuadPart - m_StartTime.QuadPart) * 1000000 / m_Frequency.QuadPart;
	// milliseconds
	return m_ElapsedTime / 1000.0f;
}

double Timer::GetDeltaTime()
{
	return m_ElapsedTime;
}
