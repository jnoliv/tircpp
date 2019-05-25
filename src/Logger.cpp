#include "Logger.h"

#include <ctime>
#include <chrono>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <unordered_map>

namespace tircpp
{
	std::mutex Logger::s_logMutex;
	std::thread Logger::s_logThread;
	std::ostream* Logger::s_logOStream;
	std::atomic<bool> Logger::s_isInitialized;
	std::queue<std::string> Logger::s_logQueue;
	std::condition_variable Logger::s_conditionVariable;

	#ifndef DEBUG
	bool Logger::initialize(const std::string&) { return false; }
	bool Logger::destroy() { return false; }
	#else

	bool Logger::initialize(const std::string& logFile)
	{
		if (s_isInitialized)
			return false;

		if (logFile.empty()) // Write to std::cout
		{
			std::lock_guard<std::mutex> lock { s_logMutex };
			s_logOStream = new std::ostream {std::cout.rdbuf() };
		}
		else // Write to given file
			s_logOStream = new std::ofstream { logFile };

		s_logThread = std::thread { &Logger::fileWriter };

		s_isInitialized = true;
		return true;
	}

	bool Logger::destroy()
	{
		if (!s_isInitialized)
			return false;

		s_isInitialized = false;

		s_conditionVariable.notify_one();
		s_logThread.join();

		{
			std::lock_guard<std::mutex> lock_guard { s_logMutex };
			delete s_logOStream;
		}

		return true;
	}
	#endif

	void Logger::log(const char* prettyFunction, std::initializer_list<std::string> list)
	{
		if (!s_isInitialized)
			return;

		std::string time;
		{
			auto nowChrono { std::chrono::system_clock::now() };
			auto nowTimeT { std::chrono::system_clock::to_time_t(nowChrono) };
			auto nowTM { *std::localtime(&nowTimeT) };

			char formattedTime[21]; // "YYYY-MM-DD HH:MM:SS.\0"
			std::strftime(formattedTime, 21, "%F %T.", &nowTM);

			auto epoch { nowChrono.time_since_epoch() };
	    	auto millis { std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count() % 1000) };

			time.reserve(24);
			time.append(formattedTime);
			time.append(std::string(3 - millis.length(), '0') + millis);
		}

		std::string tid;
		{
			auto tIdentifier { std::this_thread::get_id() };

			static uint tIndex { 0 };
			static std::mutex tMutex;
			static std::unordered_map<std::thread::id, uint> tMap;

			std::lock_guard<std::mutex> lock { tMutex };
			auto it = tMap.find(tIdentifier);
			if (it != tMap.end())
				tid = std::to_string(it->second);
			else
			{
				tMap[tIdentifier] = tIndex;
				tid = std::to_string(tIndex++);
			}

			// Left pad with zeros up to 2
			tid = std::string(2 - tid.length(), '0') + tid;
		}

		std::string function { prettyFunction };
		{
			if (function.find("lambda") != std::string::npos)
				function = "lambda";
			else
			{
				auto begin { function.find(' ') + 1 };
				auto count { function.find('(') - begin };
				function = function.substr(begin, count);
			}
		}

		size_t logsize = time.length() + tid.length() + function.length() + 10;
		for (const auto& msg : list)
			logsize += msg.size();

		std::string logMsg;
		logMsg.reserve(logsize);

		logMsg.append(time);
		logMsg.append(" | ");
		logMsg.append(tid);
		logMsg.append(" | ");
		logMsg.append(function);
		logMsg.append(" | ");

		for (const auto& msg : list)
			logMsg.append(msg);

		logMsg.append("\n");

		{
			std::lock_guard<std::mutex> lock { s_logMutex };
			s_logQueue.push(std::move(logMsg));
		}
		s_conditionVariable.notify_one();
	}

	void Logger::fileWriter()
	{
		std::unique_lock<std::mutex> lock { s_logMutex };
		lock.unlock();

		while (s_isInitialized)
		{
			lock.lock();
			s_conditionVariable.wait(lock);

			while (!s_logQueue.empty())
			{
				const auto& log { s_logQueue.front() };
				s_logOStream->write(log.c_str(), log.length());
				s_logQueue.pop();
			}

			lock.unlock();
		}
	}
}
