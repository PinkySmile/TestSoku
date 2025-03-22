//
// Created by PinkySmile on 15/03/25.
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

void modifiedPacketContent(Soku::Packet &packet)
{
	switch (packet.type) {
	case Soku::HELLO:
		packet.hello.peer.sin_port = hport;
		packet.hello.peer.sin_addr.S_addr = haddr.toInteger();
		packet.hello.target.sin_port = hport;
		packet.hello.target.sin_addr.S_addr = haddr.toInteger();
		break;
	default:
		break;
	}
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

			modifiedPacketContent(packet);
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
