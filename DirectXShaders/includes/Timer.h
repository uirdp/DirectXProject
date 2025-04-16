#pragma once

#include <Windows.h>
#include <iostream>

class Timer
{
public:
	Timer();
	void Reset();
	double GetElapsedTime();
	double GetDeltaTime();

private:
	LARGE_INTEGER m_StartTime;
	LARGE_INTEGER m_Frequency;
	LARGE_INTEGER m_LastFrameTime;
	double m_ElapsedTime;
};