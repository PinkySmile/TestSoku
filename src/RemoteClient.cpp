//
// Created by Gegel85 on 26/07/2020.
//

#ifdef _DEBUG
#include <iostream>
#include "Packet.hpp"

static void log(std::ostream &stream, const Soku::Packet &packet, bool endl = true)
{
	stream << "[CLIENT]: {";
	displayPacketContent(stream, packet);
	stream << "}";
	if (endl)
		stream << std::endl;
}
#endif
#include "RemoteClient.hpp"
#include "Exceptions.hpp"

namespace Soku
{
	void RemoteClient::_stopThread()
	{
		this->_end = true;
		if (this->_thread.joinable())
			this->_thread.join();
	}

	RemoteClient::RemoteClient(const sf::IpAddress &ip, unsigned short port)
	{
		this->setBlocking(true);
		this->setRemote(ip, port);
	}

	RemoteClient::~RemoteClient()
	{
		this->_stopThread();
	}

	void RemoteClient::sendRawData(const void *data, size_t size)
	{
		auto status = this->send(data, size, this->_ip, this->_port);

		this->_connected &= status == sf::Socket::Done;

		switch (status) {
			case sf::Socket::Status::Disconnected:
				throw DisconnectedException();
			case sf::Socket::Status::Error:
				throw SocketErrorException();
			case sf::Socket::Status::NotReady:
				throw SocketNotReadyException();
			case sf::Socket::Status::Partial:
			case sf::Socket::Status::Done:
				break;
		}
#ifdef _DEBUG
		log(std::cout, *reinterpret_cast<const Packet *>(data), false);
		std::cout << " -> " << this->_ip.toString() << ":" << this->_port << std::endl;
#endif
	}

	size_t RemoteClient::receiveNextData(void *data, size_t size)
	{
		size_t readSize;
		sf::IpAddress ip = this->_ip;
		auto port = this->_port;
		auto status = this->receive(data, size, readSize, ip, port);

		if (status == sf::Socket::Done) {
			this->_ip = ip;
			this->_port = port;
			this->_connected = true;
		} else
			this->_connected = false;

		switch (status) {
			case sf::Socket::Status::Disconnected:
				throw DisconnectedException();
			case sf::Socket::Status::Error:
				throw SocketErrorException();
			case sf::Socket::Status::NotReady:
				throw SocketNotReadyException();
			case sf::Socket::Status::Partial:
			case sf::Socket::Status::Done:
				break;
		}

		return readSize;
	}

	void RemoteClient::startThread(const std::function<void(RemoteClient &client)> &handler)
	{
		this->_stopThread();
		this->_end = false;
		this->_thread = std::thread([this, handler]{
			while (!this->_end)
				handler(*this);
		});
	}

	void RemoteClient::setRemote(const sf::IpAddress &ip, unsigned short port)
	{
		this->_port = port;
		this->_ip = ip;
	}

	const sf::IpAddress &RemoteClient::getIp() const
	{
		return this->_ip;
	}

	unsigned short RemoteClient::getPort() const
	{
		return this->_port;
	}
}