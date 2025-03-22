//
// Created by Gegel85 on 26/07/2020.
//

#ifndef SOKUTEST_REMOTECLIENT_HPP
#define SOKUTEST_REMOTECLIENT_HPP


#include <thread>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <functional>

namespace Soku
{
	class RemoteClient : sf::UdpSocket {
	private:
		sf::IpAddress _ip;
		unsigned short _port;
		std::thread _thread;
		bool _end = false;
		bool _connected = false;

		void _stopThread();

	public:
		RemoteClient() = default;
		RemoteClient(const sf::IpAddress &ip, unsigned short port);
		~RemoteClient() override;

		template<typename type>
		void sendPacket(const type &packet)
		{
			this->sendRawData(&packet, sizeof(packet));
		}

		template<typename type>
		size_t receiveNextPacket(type &packet)
		{
			puts("t");
			return this->receiveNextData(&packet, sizeof(packet));
		}

		using sf::UdpSocket::bind;
		void sendRawData(const void *data, size_t size);
		size_t receiveNextData(void *data, size_t size);
		void startThread(const std::function<void(RemoteClient &client)> &handler);
		void setRemote(const sf::IpAddress &ip, unsigned short port);
		const sf::IpAddress &getIp() const;
		unsigned short getPort() const;
		bool isConnected() const;
	};
}


#endif //SOKUTEST_REMOTECLIENT_HPP
