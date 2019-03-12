#include "CorePCH.h"
#include "GpuTimer.h"

#include <iostream>

GpuTimer::~GpuTimer()
{
	CloseLog();
}

void GpuTimer::Start(ID3D12CommandQueue * commandQueue)
{
	m_commandQueue = commandQueue ? commandQueue : CoreRender::GetInstance()->GetCommandQueue();
	
	Stop();

	if (FAILED(m_commandQueue->GetClockCalibration(&m_GPUTimer, &m_CPUTimer)))
		throw "FAILED";
}

void GpuTimer::PrintTimer()
{
	double time = GetTime();
	std::cout << "GPU Time: " << time << "s" << std::endl;
}

void GpuTimer::OpenLog(const std::string & path)
{
#ifndef _DEBUG
#ifndef _NO_LOG
	m_outSteam.open(path);

	m_threadRuning = true;
	m_threadGotWork = false;

	m_outputThread = std::thread(&GpuTimer::ThreadOutput, this);
#endif
#endif
}

void GpuTimer::CloseLog()
{
#ifndef _DEBUG
#ifndef _NO_LOG
	m_threadRuning = false;
	

	m_outSteam.close();
	if (m_outputThread.get_id() != std::thread::id())
		m_outputThread.join();
#endif
#endif
}

void GpuTimer::LogTime(const UINT & iterrationInterval)
{
#ifndef _DEBUG
#ifndef _NO_LOG
	m_iterationCounter++;

	m_avrage += GetTime();

	
	if (m_iterationCounter % iterrationInterval == 0)
	{
		m_avrage /= iterrationInterval;

		if (!m_outSteam.is_open())
			throw "Call OpenLog()";

		UpdateThread(m_avrage * 1000.0);

		m_iterationCounter = 0;
	}
#endif
#endif
}

double GpuTimer::GetTime()
{
	UINT64 endGPUTime;
	UINT64 endCPUTime;

	UINT64 frq;

	if (FAILED(m_commandQueue->GetClockCalibration(&endGPUTime, &endCPUTime)))
		throw "FAILED";
	if (FAILED(m_commandQueue->GetTimestampFrequency(&frq)))
		throw "FAILED";
		
	return (static_cast<double>(endGPUTime - m_GPUTimer) / static_cast<double>(frq));
}

void GpuTimer::Stop()
{
	m_GPUTimer = 0;
	m_CPUTimer = 0;	
}

void GpuTimer::UpdateThread(double avrage)
{

	if (!m_threadGotWork && m_threadRuning && m_outputThread.get_id() != std::thread::id())
	{
		m_printAvrage = avrage;
		m_threadGotWork = true;
	}
	else
	{
		if (m_outputThread.get_id() == std::thread::id())
		{
			m_threadRuning = true;
			m_outputThread = std::thread(&GpuTimer::ThreadOutput, this);
		}
		m_printAvrage = avrage;

		m_threadGotWork = true;
	}
}

void GpuTimer::ThreadOutput()
{
	while (m_threadRuning)
	{
		if (m_threadGotWork)
		{
			m_outSteam << m_printAvrage << std::endl;
			m_threadGotWork = false;
		}
	}
	m_threadRuning = false;
}
