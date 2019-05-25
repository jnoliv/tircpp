#include "Socket.h"

#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Logger.h"

namespace tircpp
{
	namespace
	{
		constexpr size_t READ_BUFFER_SIZE = 10240;
	}

	Socket::Socket(const std::string& url, uint16_t port, ReceiveCallback cbRecv)
		: m_port(port)
		, m_url(url)
		, m_cbRecv(cbRecv)
	{ }

	Socket::~Socket()
	{
		close(m_fileDescriptor);

		m_recvThread.join();
	}

	bool Socket::connect()
	{
		struct addrinfo hints, *p_addressList;
		{
			::memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = AF_UNSPEC;						// Allow IPv4 or IPv6
			hints.ai_socktype = SOCK_STREAM;					// Stream socket
			hints.ai_flags = (AI_V4MAPPED | AI_ADDRCONFIG);		// See man pages of getaddrinfo for explanation
		}

		auto statusCode = ::getaddrinfo(m_url.c_str(), std::to_string(m_port).c_str(), &hints, &p_addressList);
		if (statusCode != 0)
		{
			LOG(::gai_strerror(statusCode));
			return false;
		}

		struct addrinfo* i_address;
		for (i_address = p_addressList; i_address != nullptr; i_address = i_address->ai_next)
		{
			m_fileDescriptor = ::socket(i_address->ai_family, i_address->ai_socktype, i_address->ai_protocol);
			if (m_fileDescriptor == -1)
				continue;

			if (-1 != ::connect(m_fileDescriptor, i_address->ai_addr, i_address->ai_addrlen))
				break;

			close(m_fileDescriptor);
		}

		if (i_address == nullptr)
		{
			LOG("No connection could be established");
			return false;
		}

		::freeaddrinfo(p_addressList);

		LOG("Connection successfully established");

		m_recvThread = std::thread{ &Socket::onReceive, this, };

		return true;
	}

	bool Socket::send(const std::string& msg)
	{
		ssize_t writtenBytes;
		auto length{ msg.length() };
		auto p_buffer{ msg.c_str() };

		while (length > 0)
		{
			writtenBytes = ::write(m_fileDescriptor, p_buffer, length);
			if (writtenBytes == -1 && errno != EINTR)
			{
				LOG("Could not write to socket: ", std::string(::strerror(errno)))
				return false;
			}

			length -= writtenBytes;
			p_buffer += writtenBytes;
		}

		LOG("Sent: ", msg);

		return true;
	}

	void Socket::onReceive()
	{
		ssize_t readBytes;
		char p_buffer[READ_BUFFER_SIZE];

		LOG("Receiver thread started");

		struct timeval timeout;
		{
			timeout.tv_sec = 60;
			timeout.tv_usec = 0;
		}
		::setsockopt(m_fileDescriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

		while (true)
		{
			readBytes = 0;
			readBytes = ::read(m_fileDescriptor, p_buffer, READ_BUFFER_SIZE - 1);

			// If nothing was read or was interrupted by timeout
			if (readBytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
				continue;
			else if (readBytes == 0)
			{
				LOG("Socket closed locally or remotely");
				break;
			}
			else if (readBytes <= -1)
			{
				LOG("Could not read from socket: ", std::to_string(errno), " ", std::string(::strerror(errno)));
				break;
			}

			std::string msg{ p_buffer };

			if (m_cbRecv)
				m_cbRecv(msg);
		}

		LOG("Receiver thread finishing")
	}
}
