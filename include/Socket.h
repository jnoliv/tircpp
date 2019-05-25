#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <thread>
#include <string>
#include <cstdint>
#include <functional>

namespace tircpp
{
	class Socket
	{
	public:
		using ReceiveCallback = std::function<void(const std::string&)>;

		Socket(const std::string& url, uint16_t port, ReceiveCallback cbRecv);
		~Socket();

		bool connect();
		bool send(const std::string& msg);

	private:
		void onReceive();

		uint16_t m_port;
		std::string m_url;
		int m_fileDescriptor;
		ReceiveCallback m_cbRecv;
		std::thread m_recvThread;
	};
}

#endif
