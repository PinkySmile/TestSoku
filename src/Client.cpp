//
// Created by Gegel85 on 04/06/2020.
//

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include "Packet.hpp"
#include "Client.hpp"
#include "Exceptions.hpp"

namespace Soku
{
	static void log(std::ostream &stream, const Soku::Packet &packet)
	{
		stream << "[REMOTE]: {";
		displayPacketContent(stream, packet);
		stream << "}" << std::endl;
	}

	Client::Client() = default;

	Client::~Client()
	{
		this->disconnect();
	}

	void Client::disconnect()
	{
		for (auto &child : this->_children)
			this->_sendQuit(child);
		this->_sendQuit(this->_remote);
	}

	void Client::host(unsigned short port, const std::function<Inputs(State &)> &handler)
	{
		Packet packet;
		State state{true};
		sf::IpAddress ip;


	start:
		this->_remote.setRemote(sf::IpAddress::Any, port);
		this->_remote.bind(port);

		do
			this->_remote.receiveNextPacket(packet);
		while (packet.type != HELLO);

		this->_sendOlleh(this->_remote);

		do
			this->_remote.receiveNextPacket(packet);
		while (packet.type == HELLO);

		if (packet.type != INIT_REQUEST)
			goto start;

		if (packet.initRequest.reqType != PLAY_REQU) {
			this->_sendInitError(this->_remote, ERROR_GAME_STATE_INVALID);
			goto start;
		}

		std::cout << "Playing against " << std::string(packet.initRequest.name, packet.initRequest.nameLength) << std::endl;
		this->_sendInitSuccess(this->_remote, "SokuTest", packet.initRequest.name, false);

		while (!state.quit) {
			this->_remote.receiveNextPacket(packet);
			this->_handlePacket(this->_remote, state, packet, handler);
		}
	}

	void Client::connect(const std::string &host, unsigned short port)
	{
		Packet packet;
		State state{false};

		this->_handlePacket(
			this->_remote,
			state,
			this->_connect("SokuTest", host, port, false),
			{}
		);
		while (!state.quit) {
			this->_remote.receiveNextPacket(packet);
			this->_handlePacket(this->_remote, state, packet, {});
		}
	}

	void Client::connect(const std::string &host, unsigned short port, const std::function<Inputs(State &)> &handler)
	{
		Packet packet;
		State state{false};

		puts("T");
		this->_handlePacket(
			this->_remote,
			state,
			this->_connect("There is no escape", host, port, true),
			handler
		);
		while (!state.quit) {
			this->_remote.receiveNextPacket(packet);
			this->_handlePacket(this->_remote, state, packet, handler);
		}
	}

	Packet Client::_connect(const std::string &name, const std::string &host, unsigned short port, bool playing)
	{
		Packet packet;

		this->_remote.setRemote(host, port);

		SockAddrIn add;

		add.sin_family = SOCK_DGRAM;
		add.sin_port = htons(port);
		add.sin_addr.s_addr = inet_addr(this->_remote.getIp().toString().c_str());

		this->_sendHello(this->_remote, add, add);
		this->_remote.receiveNextPacket(packet);

		log(std::cout, packet);
		if (packet.type != OLLEH)
			throw InvalidHandShakeException("The host didn't respond HELLO with a ELLOH: received opcode " + std::to_string(packet.type));

		this->_sendInitRequest(this->_remote, playing ? PLAY_REQU : SPECTATE_REQU, name);
		this->_remote.receiveNextPacket(packet);

		this->_checkPacket(packet, playing);
		if (packet.type != REDIRECT)
			this->_remote.startThread([this](RemoteClient &remote){
				this->_sendChain(remote, this->_children.size());
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			});
		return packet;
	}

	void Client::_sendHello(RemoteClient &remote, const SockAddrIn &peer, const SockAddrIn &target)
	{
		PacketHello packet{
			HELLO,
			peer,
			target,
			{0, 0, 0, 0xBC}
		};

		remote.sendPacket(packet);
	}

	void Client::_sendPunch(RemoteClient &remote, const SockAddrIn &addr)
	{
		PacketPunch packet{
			PUNCH,
			addr,
			{0, 0, 0, 0}
		};

		remote.sendPacket(packet);
	}

	void Client::_sendOlleh(RemoteClient &remote)
	{
		PacketType packet{
			OLLEH
		};

		remote.sendPacket(packet);
	}

	void Client::_sendChain(RemoteClient &remote, uint32_t spectatorCount)
	{
		PacketChain packet{
			CHAIN,
			spectatorCount
		};

		//send(remote, &packet, sizeof(packet));
		remote.sendPacket(packet);
	}

	void Client::_sendInitRequest(RemoteClient &remote, RequestType type, const std::string &name)
	{
		char array[65];
		auto packet = reinterpret_cast<PacketInitRequ *>(array);
		uint8_t data[]{0x3B, 0xAA, 0x01, 0x6E, 0x28, 0x00, 0xFC, 0x30};
		unsigned char version[] = {
			0x69, 0x73, 0x65, 0xD9,
			0xFF, 0xC4, 0x6E, 0x48,
			0x8D, 0x7C, 0xA1, 0x92,
			0x31, 0x34, 0x72, 0x95
		};

		memset(array, 0, sizeof(array));
		packet->type = INIT_REQUEST;
		std::memcpy(packet->gameId, version, sizeof(packet->gameId));
		std::memcpy(packet->unknown, data, sizeof(packet->unknown));
		packet->reqType = type;

		if (type != SPECTATE_REQU) {
			std::strncpy(packet->name, name.c_str(), std::min(255U, static_cast<unsigned>(name.size())));
			packet->nameLength = strlen(packet->name);
		}
		remote.sendRawData(packet, sizeof(array));
	}

	void Client::_sendInitSuccess(RemoteClient &remote)
	{
		PacketInitSuccAdd packet{
			INIT_SUCCESS,
			{0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00},
			0,
			{0, 0}
		};

		remote.sendPacket(packet);
	}

	void Client::_sendInitSuccess(RemoteClient &remote, const std::string &hostProfileName, const std::string &clientProfileName, bool swrDisabled)
	{
		PacketInitSucc packet{
			INIT_SUCCESS,
			{0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00},
			68,
			{0, 0},
			{},
			{},
			swrDisabled
		};

		std::strncpy(packet.hostProfileName, hostProfileName.c_str(), sizeof(packet.hostProfileName));
		std::strncpy(packet.clientProfileName, clientProfileName.c_str(), sizeof(packet.clientProfileName));
		remote.sendPacket(packet);
	}

	void Client::_sendInitError(RemoteClient &remote, InitErrors error)
	{
		PacketInitError packet{
			INIT_ERROR,
			error
		};

		remote.sendPacket(packet);
	}

	void Client::_sendRedirect(RemoteClient &remote, uint32_t childId, const SockAddrIn &target)
	{
		PacketRedirect packet{
			REDIRECT,
			childId,
			target,
			{}
		};

		remote.sendPacket(packet);
	}

	void Client::_sendQuit(RemoteClient &remote)
	{
		PacketType packet{
			QUIT
		};

		remote.sendPacket(packet);
	}

	void Client::_sendGameLoaded(RemoteClient &remote, bool host, SceneId scene)
	{
		GameLoadedEvent event{
			GAME_LOADED,
			scene
		};
		PacketGame packet{
			host ? HOST_GAME : CLIENT_GAME,
			{}
		};

		packet.event.loaded = event;

		remote.sendRawData(&packet, sizeof(PacketType) + sizeof(event));
	}

	void Client::_sendGameLoadedAck(RemoteClient &remote, bool host, SceneId scene)
	{
		GameLoadedEvent event{
			GAME_LOADED_ACK,
			scene
		};
		PacketGame packet{
			host ? HOST_GAME : CLIENT_GAME,
			{}
		};

		packet.event.loaded = event;

		remote.sendRawData(&packet, sizeof(PacketType) + sizeof(event));
	}

	void Client::_sendGameInput(RemoteClient &remote, bool host, const Client::State &state, const Inputs &inputs)
	{
		size_t size = sizeof(GameInputEvent) + sizeof(PacketType) + sizeof(Inputs);
		auto *data = new uint8_t[size];
		auto *packet = reinterpret_cast<PacketGame *>(data);

		packet->type = host ? HOST_GAME : CLIENT_GAME;
		packet->event.input = {
			GAME_INPUT,
			state.frameId,
			state.currentScene,
			1,
			{}
		};
		packet->event.input.inputs[0] = inputs;

		remote.sendRawData(packet, size);
		delete[] data;
	}

	void Client::_sendGameMatch(RemoteClient &remote, bool host, const MatchData &match)
	{
		size_t size =
			sizeof(PacketType) + //Packet Header
			sizeof(GameType) + //GameEvent Header
			(sizeof(PlayerMatchData) + 1) * 2 + //Size of both players without decks
			(
				std::min(255U, static_cast<unsigned>(match.host.cards.size())) +
				std::min(255U, static_cast<unsigned>(match.client.cards.size()))
			) * sizeof(*PlayerMatchData::cards) + //Decks
			1 + 1 + 4 + 1; //Other data (<stage_id> <music_id> <random_seed> <match_id>)

		auto *data = new uint8_t[size];
		auto *packet = reinterpret_cast<PacketGame *>(data);

		packet->type = host ? HOST_GAME : CLIENT_GAME;
		packet->event.match.type = GAME_MATCH;
		packet->event.match.host.character = match.host.character;
		packet->event.match.host.skinId = match.host.skinId;
		packet->event.match.host.deckId = match.host.deckId;
		packet->event.match.host.disabledSimultaneousButton() = match.host.disabledSimultaneousButtons;
		packet->event.match.host.deckSize = match.host.cards.size();
		std::memcpy(
			packet->event.match.host.cards,
			match.host.cards.data(),
			packet->event.match.host.deckSize * sizeof(*packet->event.match.host.cards)
		);

		packet->event.match.client().character = match.client.character;
		packet->event.match.client().skinId = match.client.skinId;
		packet->event.match.client().deckId = match.client.deckId;
		packet->event.match.client().disabledSimultaneousButton() = match.client.disabledSimultaneousButtons;
		packet->event.match.client().deckSize = match.client.cards.size();
		std::memcpy(
			packet->event.match.client().cards,
			match.client.cards.data(),
			packet->event.match.host.deckSize * sizeof(*packet->event.match.client().cards)
		);

		packet->event.match.stageId() = match.stage;
		packet->event.match.musicId() = match.musicId;
		packet->event.match.randomSeed() = match.seed;
		packet->event.match.matchId() = match.matchId;

		remote.sendRawData(packet, size);
		delete[] data;
	}

	void Client::_sendGameMatchAck(RemoteClient &remote, bool host)
	{
		GameType event{
			GAME_MATCH_ACK
		};
		PacketGame packet{
			host ? HOST_GAME : CLIENT_GAME,
			{}
		};

		packet.event.type = event;

		remote.sendRawData(&packet, sizeof(PacketType) + sizeof(event));
	}

	void Client::_sendGameMatchRequest(RemoteClient &remote, bool host)
	{
		GameType event{
			GAME_MATCH_REQUEST
		};
		PacketGame packet{
			host ? HOST_GAME : CLIENT_GAME,
			{}
		};

		packet.event.type = event;

		remote.sendRawData(&packet, sizeof(PacketType) + sizeof(event));
	}

	void Client::_sendGameReplay(RemoteClient &remote, bool host, const ReplayData &data)
	{

	}

	void Client::_sendGameReplayRequest(RemoteClient &remote, bool host, uint32_t frameId, uint8_t matchId)
	{

	}

	void Client::_sendRollTime(RemoteClient &remote, uint32_t sequenceId, uint32_t timeStamp)
	{
		PacketRollTime packet{
			SOKUROLL_TIME,
			sequenceId,
			timeStamp
		};

		remote.sendPacket(packet);
	}

	void Client::_sendRollTimeAck(RemoteClient &remote, uint32_t sequenceId, uint32_t timeStamp)
	{
		PacketRollTime packet{
			SOKUROLL_TIME_ACK,
			sequenceId,
			timeStamp
		};

		remote.sendPacket(packet);
	}

	void Client::_sendRollState(RemoteClient &remote, int32_t frameId, Vector2i clientPos, Vector2i hostPos)
	{
		PacketRollState packet{
			SOKUROLL_TIME_ACK,
			frameId,
			hostPos.x,
			hostPos.y,
			clientPos.x,
			clientPos.y,
			{0, 0, 0, 0}
		};

		remote.sendPacket(packet);
	}

	void Client::_sendRollSettings(RemoteClient &remote, uint8_t maxRollback, uint8_t delay)
	{
		PacketRollSettings packet{
			SOKUROLL_SETTINGS,
			maxRollback,
			delay
		};

		remote.sendPacket(packet);
	}

	void Client::_sendRollSettingsAck(RemoteClient &remote)
	{
		PacketType packet{
			SOKUROLL_SETTINGS_ACK
		};

		remote.sendPacket(packet);
	}

	void Client::_checkPacket(const Packet &packet, bool playing)
	{
		if (packet.type != INIT_ERROR)
			return;

		switch (packet.initError.reason) {
		case ERROR_SPECTATE_DISABLED:
			throw SpectatorsNotAllowedException("Spectators are not allowed.");
		case ERROR_GAME_STATE_INVALID:
			if (playing)
				throw GameAlreadyStartedException("The game has already started.");
			throw GameNotStartedException("The game has not yet started.");
		default:
			throw UnknownErrorException("An unknown error code was received when initiating. Code " + std::to_string(packet.initError.reason) + ".");
		}
	}

	void Client::_followRedirect(const PacketRedirect &packet)
	{
		SockAddrIn add;

		add.sin_family = SOCK_DGRAM;
		add.sin_port = htons(this->_remote.getPort());
		add.sin_addr.s_addr = inet_addr(this->_remote.getIp().toString().c_str());

		this->_sendHello(this->_remote, add, packet.target);
		this->connect(inet_ntoa(packet.target.sin_addr), htons(packet.target.sin_port));
	}

	void Client::_handlePacket(RemoteClient &remote, State &state, const Packet &packet, const std::function<Inputs(State &)> &handler)
	{
#ifdef _DEBUG
		log(std::cout, packet);
#endif
		switch (packet.type) {
		case QUIT:
			state.quit = true;
			return;
		case CLIENT_GAME:
		case HOST_GAME:
			this->_treatGameEvent(remote, state, packet.game.event, handler);
			return;
		case CHAIN:
			if (state.host)
				this->_sendChain(remote, this->_children.size());
			return;
		case INIT_SUCCESS:
			std::cout << "Playing against " << packet.initSuccess.hostProfileName << " as " << packet.initSuccess.clientProfileName << std::endl;
			return;
		case SOKUROLL_TIME:
			return this->_sendRollTimeAck(remote, packet.rollTime.sequenceId, packet.rollTime.timeStamp);
		case SOKUROLL_SETTINGS:
			return this->_sendRollSettingsAck(remote);
		case REDIRECT:
			this->_followRedirect(packet.redirect);
			return;
		default:
			std::cerr << "Packet not handled: " << PacketTypeToString(packet.type) << std::endl;
			return;
		}
	}

	void Client::_treatGameEvent(RemoteClient &remote, State &state, const GameEvent &event, const std::function<Inputs(State &)> &handler)
	{
		switch (event.type) {
		case GAME_LOADED:
			std::cout << "Remote just loaded scene " << SceneIdToString(event.loaded.sceneId) << std::endl;
			state.currentScene = event.loaded.sceneId;
			this->_sendGameLoadedAck(remote, state.host, state.currentScene);
			return;
		case GAME_INPUT:
			if (handler) {
				state.frameId = event.input.frameId - 1;
				this->_sendGameInput(remote, state.host, state, handler(state));
			}
			return;
		default:
			std::cerr << "Event not handled: " << GameTypeToString(event.type) << std::endl;
			return;
		}
	}

	Client::State::State(bool hosting) :
		matchId(0),
		frameId(0),
		currentScene(SCENE_CHARACTER_SELECT),
		host(hosting),
		quit(false)
	{
	}
}