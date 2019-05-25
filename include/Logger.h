#ifndef _LOG_H_
#define _LOG_H_

#include <mutex>
#include <queue>
#include <thread>
#include <string>
#include <atomic>
#include <iostream>
#include <condition_variable>

namespace tircpp
{
	#ifdef DEBUG
	#define LOG( ... ) do { Logger::log(__PRETTY_FUNCTION__, {__VA_ARGS__}); } while (false);
	#else
	#define LOG( ... ) do {} while (false);
	#endif

	class Logger final
	{
	public:
		static bool initialize(const std::string& logFile = "");
		static bool destroy();

		static void log(const char* prettyFunction, std::initializer_list<std::string> list);

	private:
		static void fileWriter();

	private:
		static std::mutex s_logMutex;
		static std::thread s_logThread;
		static std::ostream* s_logOStream;
		static std::atomic<bool> s_isInitialized;
		static std::queue<std::string> s_logQueue;
		static std::condition_variable s_conditionVariable;
	};
}
#endif
