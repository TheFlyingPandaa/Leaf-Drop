#pragma once
#include <fstream>
#include "../Utillity/Timer.h"
class CpuTimer 
{
public:
	CpuTimer() = default;
	~CpuTimer();
	void Start();
	 
	void PrintTimer();
	void OpenLog(const std::string & path);
	void CloseLog();
	void LogTime(const UINT & iterrationInterval = 64u);

	double GetTime();
	void Stop();
private:
	Timer timer;

	UINT m_iterationCounter = 0;

	double m_avrage = 0;
	double m_printAvrage = 0;

	std::ofstream m_outSteam;

	bool m_threadGotWork = false;
	bool m_threadRuning = false;

	std::thread m_outputThread;

private:
	void UpdateThread(double avrage);
	void ThreadOutput();
};

