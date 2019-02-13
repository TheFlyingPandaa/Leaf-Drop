#pragma once
#include <fstream>

class GpuTimer 
{
public:
	~GpuTimer();
	void Start(ID3D12CommandQueue * commandQueue = nullptr);
	 
	void PrintTimer();
	void OpenLog(const std::string & path);
	void CloseLog();
	void LogTime(const UINT & iterrationInterval = 64u);

	double GetTime();
	void Stop();
private:
	ID3D12CommandQueue * m_commandQueue;

	UINT64 m_GPUTimer = 0;
	UINT64 m_CPUTimer = 0;

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

