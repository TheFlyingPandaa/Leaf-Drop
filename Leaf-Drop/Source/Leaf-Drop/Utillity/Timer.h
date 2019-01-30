#pragma once
#include <Windows.h>
#include <stdio.h>
typedef float real32;
typedef double real64;

class Timer
{
public:
	enum TIME_UNIT
	{
		SECONDS = 1,
		MILLISECONDS = 1000,
		MICROSECONDS = 1000000
	};
private:
	LARGE_INTEGER m_StartingTime;
	LARGE_INTEGER m_EndingTime;
	LARGE_INTEGER m_Frequency;
public:
	void Start()
	{
		QueryPerformanceFrequency(&m_Frequency);
		QueryPerformanceCounter(&m_StartingTime);
	}
	double Stop(TIME_UNIT tu = SECONDS)
	{
		QueryPerformanceCounter(&m_EndingTime);
		LARGE_INTEGER ElapsedMicroseconds;
		ElapsedMicroseconds.QuadPart = m_EndingTime.QuadPart - m_StartingTime.QuadPart;
		double MSPerFrame = ((real64)ElapsedMicroseconds.QuadPart / (real64)m_Frequency.QuadPart);
		m_StartingTime = m_EndingTime;
		return MSPerFrame * tu;
	}
};