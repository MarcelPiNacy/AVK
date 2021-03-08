#pragma once
#include <cstdint>
#include <atomic>
#include <thread>
#include "platform.h"
#include "algorithm_thread.h"



struct spinlock
{
	std::atomic<bool> ctrl;

	void lock();
	void unlock();
};

struct ticket_spinlock
{
	std::atomic<uint32_t> head;
	std::atomic<uint32_t> tail;

	void lock();
	void unlock();
};