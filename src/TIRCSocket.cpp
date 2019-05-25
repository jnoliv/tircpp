#include "TIRCSocket.h"

#include <cstring>

#include "Logger.h"

namespace tircpp
{
	namespace
	{
		// RFC 1459: messages shall not exceed 512 characters in length, [...] including the trailing CR-LF.
		constexpr size_t MAX_MSG_SIZE = 512;

		// RFC 1459: IRC messages are always lines of characters terminated with a CR-LF pair
		constexpr const char* MSG_DELIM = "\r\n";

		// Wrapper for std::string::find to return an iterator
		std::string::const_iterator find(const std::string& str, const std::string& pattern, std::string::const_iterator start)
		{
			size_t pos = str.find(pattern, std::distance(str.begin(), start));
			return (pos != std::string::npos) ? (str.begin() + pos) : str.end();
		}
	}

	TIRCSocket::TIRCSocket(const std::string& url, uint16_t port, Socket::ReceiveCallback cbRecv)
		: Socket(url, port, [this, cbRecv](const std::string& msg){ onReceive(msg, cbRecv); })
	{ }

	bool TIRCSocket::login(const std::string& username, const std::string& oauth)
	{
		if (!connect())
			return false;

		if (!send("PASS " + oauth))
			return false;

		if (!send("NICK " + username))
			return false;

		return true;
	}

	void TIRCSocket::onReceive(const std::string& data, Socket::ReceiveCallback cbRecv)
	{
		std::string msg;
		if (!m_partialMsg.empty())
		{
			msg = m_partialMsg;
			msg.clear();
		}

		auto msgStartIt { data.begin() };
		for (auto msgEndIt = find(data, MSG_DELIM, msgStartIt); msgEndIt != data.end();)
		{
			// The MSG_DELIM is part of the message
			msgEndIt += std::strlen(MSG_DELIM);

			// Extract message
			msg.append({ msgStartIt, msgEndIt });

			// parse msg

			if (cbRecv)
				cbRecv(msg);

			msg.clear();
			msgStartIt = msgEndIt;
			msgEndIt = find(data, MSG_DELIM, msgStartIt);
		}

		if (msgStartIt != data.end())
			m_partialMsg = std::string{ msgStartIt, data.end() };
	}

	bool TIRCSocket::send(const std::string& msg)
	{
		if (msg.size() + 2 > MAX_MSG_SIZE)
			return false;
		
		return Socket::send(msg + MSG_DELIM);
	}
}
