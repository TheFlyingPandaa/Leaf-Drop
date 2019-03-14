#include "CorePCH.h"
#include "GpuTimer.h"

#include <iostream>

GpuTimer::GpuTimer()
{
	m_frames = std::vector<TimeStamp>(NUM_FRAMES);
}

GpuTimer::~GpuTimer()
{
	m_threadRunning = false;
	m_threadDone = true;
	if (m_thread.get_id() != std::thread::id())
		m_thread.join();
}

void GpuTimer::Init()
{
	m_fence.CreateFence(L"TimeFence");
}

void GpuTimer::Start(ID3D12CommandQueue * commandQueue)
{
	if (m_fence.Done() && !m_woork)
	{
		m_fence.Signal(commandQueue);
		m_commandQueue = commandQueue;
		if (!m_commandQueue)
			return;
		m_commandQueue->GetClockCalibration(&m_GPUTimer, &m_CPUTimer);
		m_woork = true;
	}
}

void GpuTimer::Stop()
{	
	if (!m_commandQueue)
		return;

	if (m_fence.Done())
	{
		UINT64 cpu, gpu;

		m_commandQueue->GetTimestampFrequency(&m_freq);
		m_commandQueue->GetClockCalibration(&gpu, &cpu);

		TimeStamp stamp
		{
		m_CPUTimer,
		cpu,
		m_GPUTimer,
		gpu,
		((double)(gpu - m_GPUTimer) / m_freq) * 1000.0
		};

		if (counter < NUM_FRAMES)
			m_frames[counter++] = stamp;
		m_woork = false;
	}

	//if (m_threadDone && m_threadRunning && m_thread.get_id() != std::thread::id())
	//{
	//	m_threadDone = false;
	//}
	//else
	//{
	//	if (m_thread.get_id() == std::thread::id()) //if deded
	//	{
	//		m_threadRunning = true;
	//		m_thread = std::thread(&GpuTimer::_stop, this);
	//	}
	//
	//	m_threadDone = false;
	//}
}



void GpuTimer::LogTime(const std::string Path)
{
	using namespace std;

	ofstream out(Path);
	if (out.is_open())
	{
		for (size_t i = 0; i < m_frames.size(); i++)
		{
			out << m_frames.at(i).toString() << "\n";
		}
		out.close();
	}
}

void GpuTimer::_stop()
{
	while (m_threadRunning)
	{
		if (!m_threadDone)
		{
			if (m_fence.Done())
			{
				UINT64 cpu, gpu;

				m_commandQueue->GetTimestampFrequency(&m_freq);
				m_commandQueue->GetClockCalibration(&gpu, &cpu);

				TimeStamp stamp
				{
				m_CPUTimer,
				cpu,
				m_GPUTimer,
				gpu,
				((double)(gpu - m_GPUTimer) / m_freq) * 1000.0
				};

				if (counter < NUM_FRAMES)
					m_frames[counter++] = stamp;
				m_threadDone = true;
			}
		}
	}
	m_threadRunning = false;
}



