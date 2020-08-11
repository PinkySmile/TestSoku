//
// Created by Gegel85 on 06/06/2020.
//

#include <iomanip>
#include <iostream>
#include <fstream>
#include <SFML/Network/UdpSocket.hpp>
#include <inaddr.h>
#include "Packet.hpp"

unsigned short hostPort = 10800;
sf::IpAddress hostAddress = "localhost";
bool mutex = false;

void displayGameEvent(std::ostream &stream, Soku::GameEvent &event)
{
	stream << ", eventType: " << GameTypeToString(event.type);
	switch (event.type) {
	case Soku::GAME_LOADED:
	case Soku::GAME_LOADED_ACK:
		stream << ", sceneId: " << SceneIdToString(event.loaded.sceneId);
		break;
	case Soku::GAME_INPUT:
		stream << ", frameId: " << event.input.frameId;
		stream << ", sceneId: " << SceneIdToString(event.input.sceneId);
		stream << ", inputCount: " << static_cast<int>(event.input.inputCount);
		stream << ", inputs: [";
		for (int i = 0; i < event.input.inputCount; i++)
			stream << (i == 0 ? "" : ", ") << *reinterpret_cast<uint16_t *>(&event.input.inputs[i]);
		stream << "]";
		break;
	case Soku::GAME_MATCH:
		break;
	case Soku::GAME_MATCH_ACK:
		break;
	case Soku::GAME_MATCH_REQUEST:
		break;
	case Soku::GAME_REPLAY:
		break;
	case Soku::GAME_REPLAY_REQUEST:
		break;
	}
}

void displayPacketContent(std::ostream &stream, Soku::Packet &packet)
{
	switch (packet.type) {
	case Soku::HELLO:
		stream << ", peer: ";
		stream << static_cast<int>(packet.hello.peer.sin_addr.S_un.S_un_b.s_b1) << ".";
		stream << static_cast<int>(packet.hello.peer.sin_addr.S_un.S_un_b.s_b2) << ".";
		stream << static_cast<int>(packet.hello.peer.sin_addr.S_un.S_un_b.s_b3) << ".";
		stream << static_cast<int>(packet.hello.peer.sin_addr.S_un.S_un_b.s_b4) << ":";
		stream << static_cast<int>(htons(packet.hello.peer.sin_port)) << "}, target: ";
		packet.hello.peer.sin_port = hostPort;
		packet.hello.peer.sin_addr.S_un.S_addr = hostAddress.toInteger();
		stream << static_cast<int>(packet.hello.target.sin_addr.S_un.S_un_b.s_b1) << ".";
		stream << static_cast<int>(packet.hello.target.sin_addr.S_un.S_un_b.s_b2) << ".";
		stream << static_cast<int>(packet.hello.target.sin_addr.S_un.S_un_b.s_b3) << ".";
		stream << static_cast<int>(packet.hello.target.sin_addr.S_un.S_un_b.s_b4) << ":";
		stream << static_cast<int>(htons(packet.hello.target.sin_port)) << ", unknown: [";
		packet.hello.target.sin_port = hostPort;
		packet.hello.target.sin_addr.S_un.S_addr = hostAddress.toInteger();
		stream << std::hex;
		for (int i = 0; i < 4; i++)
			stream << (i == 0 ? "" : ", ") << "0x" << static_cast<int>(packet.punch.unknown[i]);
		stream << "]" << std::dec;
		break;
	case Soku::PUNCH:
		stream << ", addr: ";
		stream << static_cast<int>(packet.punch.addr.sin_addr.S_un.S_un_b.s_b1) << ".";
		stream << static_cast<int>(packet.punch.addr.sin_addr.S_un.S_un_b.s_b2) << ".";
		stream << static_cast<int>(packet.punch.addr.sin_addr.S_un.S_un_b.s_b3) << ".";
		stream << static_cast<int>(packet.punch.addr.sin_addr.S_un.S_un_b.s_b4) << ":";
		stream << static_cast<int>(htons(packet.punch.addr.sin_port)) << ", unknown: [";
		stream << std::hex;
		for (int i = 0; i < 4; i++)
			stream << (i == 0 ? "" : ", ") << "0x" << static_cast<int>(packet.punch.unknown[i]);
		stream << "]" << std::dec;
		break;
	case Soku::CHAIN:
		stream << ", spectatorCount: " << packet.chain.spectatorCount;
		break;
	case Soku::INIT_REQUEST:
		stream << ", gameId: [" << std::hex;
		for (int i = 0; i < 16; i++)
			stream << (i == 0 ? "" : ", ") << "0x" << static_cast<int>(packet.initRequest.gameId[i]);
		stream << "], unknown: [";
		for (int i = 0; i < 8; i++)
			stream << (i == 0 ? "" : ", ") << "0x" << static_cast<int>(packet.initRequest.unknown[i]);
		stream << "], reqType: " << RequestTypeToString(packet.initRequest.reqType) << std::dec;
		stream << ", nameLength: " << static_cast<int>(packet.initRequest.nameLength) << ", name: \"";
		stream << std::string(packet.initRequest.name, packet.initRequest.nameLength) << "\"";
		break;
	case Soku::INIT_SUCCESS:
		stream << ", unknown1: [";
		stream << std::hex;
		for (int i = 0; i < 8; i++)
			stream << (i == 0 ? "" : ", ") << "0x" << static_cast<int>(packet.initSuccess.unknown1[i]);
		stream << "], dataSize: " << std::dec << static_cast<int>(packet.initSuccess.dataSize);
		stream << ", unknown2: [" << std::hex;
		for (int i = 0; i < 3; i++)
			stream << (i == 0 ? "" : ", ") << "0x" << static_cast<int>(packet.initSuccess.unknown2[i]);
		stream << "]" << std::dec;
		if (packet.initSuccess.dataSize) {
			stream << ", hostProfileName: \"" << packet.initSuccess.hostProfileName << "\"";
			stream << ", clientProfileName: \"" << packet.initSuccess.clientProfileName << "\"";
			stream << ", swrDisabled: " << packet.initSuccess.swrDisabled;
		}
		break;
	case Soku::INIT_ERROR:
		stream << ", reason: " << packet.initError.reason;
		break;
	case Soku::REDIRECT:
		stream << ", childId: " << packet.redirect.childId;
		stream << ", target: ";
		stream << static_cast<int>(packet.redirect.target.sin_addr.S_un.S_un_b.s_b1) << ".";
		stream << static_cast<int>(packet.redirect.target.sin_addr.S_un.S_un_b.s_b2) << ".";
		stream << static_cast<int>(packet.redirect.target.sin_addr.S_un.S_un_b.s_b3) << ".";
		stream << static_cast<int>(packet.redirect.target.sin_addr.S_un.S_un_b.s_b4) << ":";
		stream << static_cast<int>(htons(packet.redirect.target.sin_port)) << ", unknown: [" << std::hex;
		for (int i = 0; i < 48; i++)
			stream << (i == 0 ? "" : ", ") << "0x" << static_cast<int>(packet.redirect.unknown[i]);
		stream << "]" << std::dec;
		break;
	case Soku::OLLEH:
	case Soku::QUIT:
	case Soku::SOKUROLL_SETTINGS_ACK:
		break;
	case Soku::HOST_GAME:
	case Soku::CLIENT_GAME:
		displayGameEvent(stream, packet.game.event);
		break;
	case Soku::SOKUROLL_TIME:
	case Soku::SOKUROLL_TIME_ACK:
		stream << ", sequenceId: " << packet.rollTime.sequenceId;
		stream << ", timeStamp: " << packet.rollTime.timeStamp;
		break;
	case Soku::SOKUROLL_STATE:
		stream << ", frameId: " << packet.rollState.frameId;
		stream << ", hostX: " << packet.rollState.hostX;
		stream << ", hostY: " << packet.rollState.hostY;
		stream << ", clientX: " << packet.rollState.clientX;
		stream << ", clientY: " << packet.rollState.clientY;
		stream << ", stuff: [" << std::hex;
		for (int i = 0; i < 4; i++)
			stream << (i == 0 ? "" : ", ") << "0x" << static_cast<int>(packet.rollState.stuff[i]);
		stream << "]" << std::dec;
		break;
	case Soku::SOKUROLL_SETTINGS:
		stream << ", maxRollback: " << static_cast<int>(packet.rollSettings.maxRollback);
		stream << ", delay: " << static_cast<int>(packet.rollSettings.delay);
		break;
	}
}

void log(std::ostream &stream, Soku::Packet &packet, bool host)
{
	while (mutex);
	mutex = true;

	stream << (host ? "[HOST]:   " : "[CLIENT]: ");
	stream << "{type: " << PacketTypeToString(packet.type);
	displayPacketContent(stream, packet);
	stream << "}" << std::endl;

	mutex = false;
}

int main()
{
	std::ofstream file{"log.txt"};
	sf::IpAddress clientAddress = sf::IpAddress::Any;
	unsigned short clientPort = 10801;
	sf::UdpSocket hostSock;
	sf::UdpSocket clientSock;
	Soku::Packet packet;
	size_t size;
	Soku::SockAddrIn add;

	add.sin_family = SOCK_DGRAM;
	add.sin_port = htons(hostPort);
	add.sin_addr.S_un.S_addr = hostAddress.toInteger();

	Soku::PacketHello p{
		Soku::HELLO,
		add,
		add,
		{0, 0, 0, 0xBC}
	};

	clientSock.bind(clientPort);
	hostSock.setBlocking(false);
	hostSock.send(&p, sizeof(p), hostAddress, hostPort);
	while (true) {
		if (hostSock.receive(&packet, sizeof(packet), size, hostAddress, hostPort) == sf::Socket::Done) {
			log(file, packet, true);
			clientSock.send(&packet, size, clientAddress, clientPort);
		}

		if (clientSock.receive(&packet, sizeof(packet), size, clientAddress, clientPort) == sf::Socket::Done) {
			log(file, packet, false);
			hostSock.send(&packet, size, hostAddress, hostPort);
		}
	}
}