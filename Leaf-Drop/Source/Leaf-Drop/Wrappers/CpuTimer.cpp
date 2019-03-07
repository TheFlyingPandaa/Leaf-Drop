#include "CorePCH.h"
#include "CpuTimer.h"

#include <iostream>

CpuTimer::~CpuTimer()
{
	CloseLog();
}

void CpuTimer::Start()
{		
	timer.Start();
}

void CpuTimer::PrintTimer()
{
	double time = GetTime();
	std::cout << "GPU Time: " << time << "s" << std::endl;
}

void CpuTimer::OpenLog(const std::string & path)
{
	m_outSteam.open(path);

	m_threadRuning = true;
	m_threadGotWork = false;

	m_outputThread = std::thread(&CpuTimer::ThreadOutput, this);
}

void CpuTimer::CloseLog()
{
	m_threadRuning = false;
	

	m_outSteam.close();
	if (m_outputThread.get_id() != std::thread::id())
		m_outputThread.join();
}

void CpuTimer::LogTime(const UINT & iterrationInterval)
{
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
}

double CpuTimer::GetTime()
{
	return timer.Stop(Timer::MILLISECONDS);
}

void CpuTimer::Stop()
{
	timer.Stop();
}

void CpuTimer::UpdateThread(double avrage)
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
			m_outputThread = std::thread(&CpuTimer::ThreadOutput, this);
		}
		m_printAvrage = avrage;

		m_threadGotWork = true;
	}
}

void CpuTimer::ThreadOutput()
{
	while (m_threadRuning)
	{
		if (m_threadGotWork)
		{
			m_outSteam << m_avrage << std::endl;
			m_threadGotWork = false;
		}
	}
	m_threadRuning = false;
}
