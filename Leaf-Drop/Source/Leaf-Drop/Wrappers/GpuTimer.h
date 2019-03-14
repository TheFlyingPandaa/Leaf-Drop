#pragma once
#include <fstream>
#include "../Wrappers/Fence.h"


class GpuTimer 
{
public:
	GpuTimer();
	~GpuTimer();
	void Start(ID3D12CommandQueue * commandQueue);
	void Stop();

	void Init();

	void LogTime(const std::string Path);

private:

	void _stop();

	ID3D12CommandQueue * m_commandQueue = nullptr;
	UINT64 m_freq = 0; 
	UINT64 m_GPUTimer = 0;
	UINT64 m_CPUTimer = 0;

	bool m_woork = false;


	struct TimeStamp
	{
		UINT64 CPUBegin;
		UINT64 CPUEnd;
		UINT64 GPUBegin;
		UINT64 GPUEnd;
		double deltaTime;

		std::string toString()
		{
			return std::to_string(CPUBegin) + "\t" +
				std::to_string(CPUEnd) + "\t" +
				std::to_string(GPUBegin) + "\t" +
				std::to_string(GPUEnd) + "\t" +
				std::to_string(deltaTime);
		}
	};

	UINT64 counter = 0;
	std::vector<TimeStamp> m_frames;
	Fence m_fence;

	bool m_threadRunning = false;
	bool m_threadDone = false;
	std::thread m_thread;

};

