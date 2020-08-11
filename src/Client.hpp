//
// Created by Gegel85 on 04/06/2020.
//

#ifndef SOKUTEST_CLIENT_HPP
#define SOKUTEST_CLIENT_HPP


#include <SFML/Network.hpp>
#include <functional>
#include <thread>
#include "Vector2.hpp"
#include "Packet.hpp"
#include "RemoteClient.hpp"

namespace Soku
{
	class Client {
	public:
		struct State {
			uint32_t rollSequenceId = 1;
			uint32_t matchId;
			uint32_t frameId;
			SceneId currentScene;
			bool host;
			bool spectating;
			bool quit;
			std::string myName;
			std::string opponentName;
			MatchData match;

			State(bool hosting);
		};

	private:
		std::vector<RemoteClient> _children;
		RemoteClient _remote;

		void _followRedirect(const PacketRedirect &packet);
		void _sendHello(RemoteClient &remote, const SockAddrIn &peer, const SockAddrIn &target);
		void _sendPunch(RemoteClient &remote, const SockAddrIn &addr);
		void _sendOlleh(RemoteClient &remote);
		void _sendChain(RemoteClient &remote, uint32_t spectatorCount);
		void _sendInitRequest(RemoteClient &remote, RequestType type, const std::string &name);
		void _sendInitSuccess(RemoteClient &remote);
		void _sendInitSuccess(RemoteClient &remote, const std::string &hostProfileName, const std::string &clientProfileName, bool swrDisabled);
		void _sendInitError(RemoteClient &remote, InitErrors error);
		void _sendRedirect(RemoteClient &remote, uint32_t childId, const SockAddrIn &target);
		void _sendQuit(RemoteClient &remote);
		void _sendGameLoaded(RemoteClient &remote, bool host, SceneId scene);
		void _sendGameLoadedAck(RemoteClient &remote, bool host, SceneId scene);
		void _sendGameInput(RemoteClient &remote, bool host, const State &state, const Inputs &inputs);
		void _sendGameMatch(RemoteClient &remote, bool host, const MatchData &match);
		void _sendGameMatchAck(RemoteClient &remote, bool host);
		void _sendGameMatchRequest(RemoteClient &remote, bool host);
		void _sendGameReplay(RemoteClient &remote, bool host, const ReplayData &data);
		void _sendGameReplayRequest(RemoteClient &remote, bool host, uint32_t frameId, uint8_t matchId);
		void _sendRollTime(RemoteClient &remote, uint32_t sequenceId, uint32_t timeStamp);
		void _sendRollTimeAck(RemoteClient &remote, uint32_t sequenceId, uint32_t timeStamp);
		void _sendRollState(RemoteClient &remote, int32_t frameId, Vector2i clientPos, Vector2i hostPos);
		void _sendRollSettings(RemoteClient &remote, uint8_t maxRollback, uint8_t delay);
		void _sendRollSettingsAck(RemoteClient &remote);

		void _checkPacket(const Packet &packet, bool playing);

		void _handlePacket(RemoteClient &remote, State &state, const Packet &packet, const std::function<Inputs(State &)> &handler);
		void _treatGameEvent(RemoteClient &remote, State &state, const GameEvent &event, const std::function<Inputs(State &)> &handler);

		Packet _connect(const std::string &name, const std::string &host, unsigned short port, bool playing);

	public:
		Client();
		~Client();
		void host(unsigned short port, const std::function<Inputs(State &)> &handler);
		void disconnect();
		void connect(const std::string &host, unsigned short port);
		void connect(const std::string &host, unsigned short port, const std::function<Inputs(State &)> &handler);
	};
}


#endif //SOKUTEST_CLIENT_HPP
