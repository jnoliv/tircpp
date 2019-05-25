#include <cstdint>
#include <iostream>

#include "Logger.h"
#include "TIRCSocket.h"

using namespace tircpp;

namespace
{
	constexpr uint16_t TWITCH_CHAT_PORT = 6667;
	constexpr const char* TWITCH_CHAT_URL = "irc.chat.twitch.tv";
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << "Usage: tirc++_test <twitch_username> <oauth_token>" << std::endl;
		return -1;
	}

	Logger::initialize();

	{
		TIRCSocket sock{TWITCH_CHAT_URL, TWITCH_CHAT_PORT,
		[&sock](const std::string& msg)
		{
			LOG("Received: ", msg);

			/*static uint msgNumber = 0;
			if (msgNumber == 0)
				sock.send("CAP REQ :twitch.tv/membership\r\n");
			else if (msgNumber == 1)
				sock.send("JOIN #" + std::string(TWITCH_CHANNEL) + "\r\n");
			else if (msgNumber == 2)
				sock.send("PRIVMSG #" + std::string(TWITCH_CHANNEL) + " :" + "Test message, please ignore\r\n");

			//sock.send("PART #" + std::string(TWITCH_CHANNEL));
			++msgNumber;*/
		}};
		sock.login(argv[1], argv[2]);

		std::string dummy;
		std::cin >> dummy;

		sock.send("QUIT\r\n");
	}

	LOG("Exiting...");

	Logger::destroy();

	return 0;
}
