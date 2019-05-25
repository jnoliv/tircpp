#ifndef _TIRCSOCKET_H_
#define _TIRCSOCKET_H_

#include "Socket.h"

namespace tircpp
{
	class TIRCSocket : private Socket
	{
	public:
		TIRCSocket(const std::string& url, uint16_t port, Socket::ReceiveCallback cbRecv);
		~TIRCSocket() = default;

		bool login(const std::string& username, const std::string& oauth);
		bool send(const std::string& msg);

	private:
		void onReceive(const std::string& data, Socket::ReceiveCallback cbRecv);

		std::string m_partialMsg;
	};
}

#endif
