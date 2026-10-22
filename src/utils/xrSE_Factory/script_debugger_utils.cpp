#include "stdafx.h"

#include "script_debugger_utils.h"

#include <chrono>

namespace
{
	// A steady clock that does not jump when the system time changes; the Win32 counterpart
	// (GetTickCount64) has the same property.
	const std::chrono::steady_clock::time_point g_start = std::chrono::steady_clock::now();
}

xr_event::xr_event(bool broadcast, bool signalled)
	: m_broadcast(broadcast)
	, m_signalled(signalled)
{
}

xr_event::~xr_event()
{
}

bool xr_event::signal()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_signalled = true;
	}

	// A manual-reset event wakes every waiter; an auto-reset one releases a single waiter.
	if (m_broadcast)
		m_condition.notify_all();
	else
		m_condition.notify_one();

	return true;
}

bool xr_event::reset()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_signalled = false;
	return true;
}

xrEventWaitRes xr_event::wait()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_condition.wait(lock, [this] { return m_signalled; });

	if (!m_broadcast)
		m_signalled = false;

	return wrSignaled;
}

xrEventWaitRes xr_event::wait(unsigned msec)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!m_condition.wait_for(lock, std::chrono::milliseconds(msec), [this] { return m_signalled; }))
		return wrTimeOut;

	if (!m_broadcast)
		m_signalled = false;

	return wrSignaled;
}
////////////////////////////////////////////////////////////

xr_mutex::xr_mutex()
{
}

xr_mutex::~xr_mutex()
{
}

bool xr_mutex::trylock()
{
	return m_mutex.try_lock();
}

bool xr_mutex::lock()
{
	m_mutex.lock();
	return true;
}

bool xr_mutex::unlock()
{
	m_mutex.unlock();
	return true;
}

xr_sync::xr_sync(xr_mutex & mutex)
: m_mutex(mutex)
{
	m_mutex.lock();
}

xr_sync::~xr_sync()
{
	m_mutex.unlock();
}

////////////////////////////////////////////////////////////

xr_thread::xr_thread()
{
}

xr_thread::~xr_thread()
{
	if (m_thread.joinable())
		m_thread.detach();
}

bool xr_thread::start()
{
	kill();
	m_thread = std::thread(&xr_thread::run, this);
	return m_thread.joinable();
}

bool xr_thread::kill()
{
	if (!m_thread.joinable())
		return true;

	// There is no portable way to terminate a running thread. Nothing in the debugger force
	// kills a live thread, so the handle is detached and the thread finishes on its own.
	m_thread.detach();
	return true;
}

bool xr_thread::join()
{
	if (!m_thread.joinable())
		return false;

	m_thread.join();
	return true;
}

bool xr_thread::yield()
{
	std::this_thread::yield();
	return true;
}

void xr_thread::sleep(unsigned msec)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(msec));
}

unsigned xr_thread::getTickCount()
{
	const auto elapsed = std::chrono::steady_clock::now() - g_start;
	return static_cast<unsigned>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}


xr_waitableThread::xr_waitableThread()
: m_event(true, true)
{}

xr_waitableThread::~xr_waitableThread()
{}

bool xr_waitableThread::start()
{
	m_event.reset();

	if (xr_thread::start())
		return true;
	else
	{
		m_event.signal();
		return false;
	}
}

bool xr_waitableThread::kill()
{
	if (xr_thread::kill())
	{
		m_event.signal();
		return true;
	}
	else
		return false;
}

bool xr_waitableThread::join()
{
	return xr_thread::join();
}

bool xr_waitableThread::join(unsigned msec)
{
	return m_event.wait(msec) == wrSignaled;
}

bool xr_waitableThread::yield()
{
	return xr_thread::yield();
}

void xr_waitableThread::run()
{
	run_w();
	m_event.signal();
}
