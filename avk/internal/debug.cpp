#include "debug.h"
#include "locks.h"
#include <Windows.h>

#ifdef DEBUG
namespace debug
{
	ticket_spinlock io_lock;

	void log(const char* message)
	{
		io_lock.lock();
		OutputDebugStringA(message);
		OutputDebugStringA("\n");
		io_lock.unlock();
	}

	void log(std::string_view message)
	{
		if (message.size() < 4096)
		{
			char* buffer = (char*)alloca(message.size() + 1);
			(void)memcpy(buffer, message.data(), message.size());
			buffer[message.size()] = '\0';
			log(buffer);
		}
		else
		{
			char* buffer = (char*)malloc(message.size() + 1);
			AVK_ASSERT(buffer != nullptr);
			(void)memcpy(buffer, message.data(), message.size());
			buffer[message.size()] = '\0';
			log(buffer);
			free(buffer);
		}
	}
}
#endif