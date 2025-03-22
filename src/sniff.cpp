//
// Created by Gegel85 on 06/06/2020.
//

#include <iomanip>
#include <iostream>
#include <fstream>
#include <thread>
#include <memory>
#include <mutex>
#include <random>
#include <SFML/Network/UdpSocket.hpp>
#include "Packet.hpp"
#ifdef _WIN32
#include <inaddr.h>
#define s_b1 S_un.S_un_b.s_b1
#define s_b2 S_un.S_un_b.s_b2
#define s_b3 S_un.S_un_b.s_b3
#define s_b4 S_un.S_un_b.s_b4
#define S_addr S_un.S_addr
#else
#include <arpa/inet.h>
#define s_b1 s_addr >> 0 & 0xFF
#define s_b2 s_addr >> 8 & 0xFF
#define s_b3 s_addr >> 16 & 0xFF
#define s_b4 s_addr >> 24 & 0xFF
#define S_addr s_addr
#endif

std::mutex mutex;
sf::IpAddress haddr;
unsigned short hport;

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
		stream << ", inputs: [" << std::hex;
		for (int i = 0; i < event.input.inputCount; i++)
			stream << (i == 0 ? "" : ", ") << *reinterpret_cast<const uint16_t *>(&event.input.inputs[i]);
		stream << "]" << std::dec;
		break;
	case Soku::GAME_MATCH:
		stream << ", host: " << event.match.host;
		stream << ", client: " << event.match.client();
		stream << ", stageId: " << +event.match.stageId();
		stream << ", musicId: " << +event.match.musicId();
		stream << ", randomSeed: " << event.match.randomSeed();
		stream << ", matchId: " << +event.match.matchId();
		break;
	case Soku::GAME_REPLAY:
		stream << ", replaySize: " << +event.replay.replaySize;
		break;
	case Soku::GAME_REPLAY_REQUEST:
		stream << ", frameId: " << +event.replayRequest.frameId;
		stream << ", matchId: " << +event.replayRequest.matchId;
		break;
	case Soku::GAME_MATCH_ACK:
	case Soku::GAME_MATCH_REQUEST:
		break;
	}
}

void displayPacketContent(std::ostream &stream, Soku::Packet &packet)
{
	switch (packet.type) {
	case Soku::HELLO:
		stream << ", peer: ";
		stream << static_cast<int>(packet.hello.peer.sin_addr.s_b1) << ".";
		stream << static_cast<int>(packet.hello.peer.sin_addr.s_b2) << ".";
		stream << static_cast<int>(packet.hello.peer.sin_addr.s_b3) << ".";
		stream << static_cast<int>(packet.hello.peer.sin_addr.s_b4) << ":";
		stream << static_cast<int>(htons(packet.hello.peer.sin_port)) << ", target: ";
		packet.hello.peer.sin_port = hport;
		packet.hello.peer.sin_addr.S_addr = haddr.toInteger();
		stream << static_cast<int>(packet.hello.target.sin_addr.s_b1) << ".";
		stream << static_cast<int>(packet.hello.target.sin_addr.s_b2) << ".";
		stream << static_cast<int>(packet.hello.target.sin_addr.s_b3) << ".";
		stream << static_cast<int>(packet.hello.target.sin_addr.s_b4) << ":";
		stream << static_cast<int>(htons(packet.hello.target.sin_port)) << ", unknown: [";
		packet.hello.target.sin_port = hport;
		packet.hello.target.sin_addr.S_addr = haddr.toInteger();
		stream << std::hex;
		for (int i = 0; i < 4; i++)
			stream << (i == 0 ? "" : ", ") << "0x" << static_cast<int>(packet.punch.unknown[i]);
		stream << "]" << std::dec;
		break;
	case Soku::PUNCH:
		stream << ", addr: ";
		stream << static_cast<int>(packet.punch.addr.sin_addr.s_b1) << ".";
		stream << static_cast<int>(packet.punch.addr.sin_addr.s_b2) << ".";
		stream << static_cast<int>(packet.punch.addr.sin_addr.s_b3) << ".";
		stream << static_cast<int>(packet.punch.addr.sin_addr.s_b4) << ":";
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
		stream << static_cast<int>(packet.redirect.target.sin_addr.s_b1) << ".";
		stream << static_cast<int>(packet.redirect.target.sin_addr.s_b2) << ".";
		stream << static_cast<int>(packet.redirect.target.sin_addr.s_b3) << ".";
		stream << static_cast<int>(packet.redirect.target.sin_addr.s_b4) << ":";
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
	mutex.lock();

	stream << (host ? "[HOST]:   " : "[CLIENT]: ");
	stream << "{type: " << PacketTypeToString(packet.type);
	displayPacketContent(stream, packet);
	stream << "}" << std::endl;

	mutex.unlock();
}

//127.0.0.1:10801
int main(int argc, char **argv)
{
	if (argc < 5) {
		std::cout << "Usage: " << argv[0] << " <bind_port> <remote_ip> <remote_port> <log>" << std::endl;
		return EXIT_FAILURE;
	}

	std::ofstream file{argv[4]};
	sf::UdpSocket sock;
	sf::IpAddress caddr = sf::IpAddress::Any;
	unsigned short cport = 0;
	float packet_lost = 0;
	size_t size;
	Soku::SockAddrIn add;
	std::uniform_real_distribution<float> loss_dist{0, 1};
	std::uniform_int_distribution<uint64_t> delay_dist{0, 0};
	std::mt19937_64 random_gen{static_cast<unsigned long>(time(nullptr))};

	haddr = sf::IpAddress{argv[2]};
	hport = std::stoul(argv[3]);

	add.sin_family = SOCK_DGRAM;
	add.sin_port = htons(hport);
	add.sin_addr.S_addr = haddr.toInteger();

	Soku::PacketHello p{
		Soku::HELLO,
		add,
		add,
		{0, 0, 0, 0xBC}
	};

	if (sock.bind(std::stoul(argv[1])) != sf::Socket::Done)
		return EXIT_FAILURE;
	sock.setBlocking(false);
	sock.send(&p, sizeof(p), haddr, hport);
	std::thread{[&delay_dist, &packet_lost]{
		std::string s;

		std::cout << "> ";
		while (std::getline(std::cin, s)) {
			size_t pos1 = s.find(':');
			size_t pos2 = s.find('/');

			try {
				if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 > pos2)
					throw std::exception();

				std::string pl = s.substr(0, pos1);
				std::string nd = s.substr(pos1 + 1, pos2 - pos1 - 1);
				std::string xd = s.substr(pos2 + 1);
				auto ndf = std::stoul(nd);
				auto xdf = std::stoul(xd);

				if (ndf > xdf)
					throw std::exception();
				packet_lost = std::stof(pl);
				delay_dist = std::uniform_int_distribution<uint64_t>{ndf * 1000, xdf * 1000};
				std::cout << "Updated settings to " << packet_lost * 100 << "% packet loss, delay between " << ndf << "ms and " << xdf << "ms" << std::endl;
			} catch (...) {
				std::cout << "Invalid format. Expected <packetloss>:<mindelay>/<maxdelay>" << std::endl;
			}
			std::cout << "> ";
		}
		exit(EXIT_SUCCESS);
	}}.detach();
	while (true) {
		auto buffer = std::shared_ptr<char>((char *)malloc(1024 * 1024), free);
		size_t total = 0;
		sf::IpAddress addr = sf::IpAddress::Any;
		unsigned short port = 0;
		auto &packet = *(Soku::Packet *)&*buffer;

		if (sock.receive(&*buffer, 1024 * 1024, total, addr, port) == sf::Socket::Done) {
			auto t = delay_dist(random_gen);
			sf::IpAddress raddr = addr == haddr && port == hport ? caddr : haddr;
			unsigned short rport = addr == haddr && port == hport ? cport : hport;

			log(file, packet, addr == haddr && port == hport);
			log(std::cout, packet, addr == haddr && port == hport);
			if ((addr != haddr || port != hport) && cport == 0) {
				caddr = addr;
				cport = port;
			}
			if (loss_dist(random_gen) < packet_lost)
				continue;
			std::thread{[t, buffer, total, raddr, rport, &sock] {
				std::this_thread::sleep_for(std::chrono::microseconds(t));
				sock.send(&*buffer, total, raddr, rport);
			}}.detach();
		}
	}
}
