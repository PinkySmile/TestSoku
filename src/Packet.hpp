//
// Created by Gegel85 on 04/06/2020.
//

#ifndef SOKUTEST_PACKET_HPP
#define SOKUTEST_PACKET_HPP

#include <cstdint>
#ifdef _WIN32
#include <winsock.h>
#include <vector>

#else
#include <netinet/in.h>
#endif


namespace Soku
{
	#define PACKED __attribute__((__packed__))

	extern uint8_t Soku110acRollSWRAllChars[16];
	extern uint8_t Soku110acNoRollSWRAllChars[16];
	extern uint8_t Soku110acNoSWRAllChars[16];

	enum PACKED PacketType {
		HELLO = 0x01,
		PUNCH,
		OLLEH,
		CHAIN,
		INIT_REQUEST,
		INIT_SUCCESS,
		INIT_ERROR,
		REDIRECT,
		QUIT = 0x0B,
		HOST_GAME = 0x0D,
		CLIENT_GAME,
		SOKUROLL_TIME = 0x10,
		SOKUROLL_TIME_ACK,
		SOKUROLL_STATE,
		SOKUROLL_SETTINGS,
		SOKUROLL_SETTINGS_ACK,
	};

	enum PACKED RequestType {
		SPECTATE_REQU,
		PLAY_REQU
	};

	enum PACKED SceneId {
		SCENE_CHARACTER_SELECT = 0x03,
		SCENE_BATTLE = 0x05,
	};

	enum InitErrors {
		ERROR_SPECTATE_DISABLED,
		ERROR_GAME_STATE_INVALID
	};

	enum PACKED Character {
		CHARACTER_REIMU,
		CHARACTER_MARISA,
		CHARACTER_SAKUYA,
		CHARACTER_ALICE,
		CHARACTER_PATCHOULI,
		CHARACTER_YOUMU,
		CHARACTER_REMILIA,
		CHARACTER_YUYUKO,
		CHARACTER_YUKARI,
		CHARACTER_SUIKA,
		CHARACTER_REISEN,
		CHARACTER_AYA,
		CHARACTER_KOMACHI,
		CHARACTER_IKU,
		CHARACTER_TENSHI,
		CHARACTER_SANAE,
		CHARACTER_CIRNO,
		CHARACTER_MEILING,
		CHARACTER_UTSUHO,
		CHARACTER_SUWAKO,
	};

	enum PACKED GameType {
		GAME_LOADED = 0x01,
		GAME_LOADED_ACK,
		GAME_INPUT,
		GAME_MATCH,
		GAME_MATCH_ACK,
		GAME_MATCH_REQUEST = 0x08,
		GAME_REPLAY,
		GAME_REPLAY_REQUEST = 0x0B
	};

	typedef struct sockaddr_in SockAddrIn;

	struct PACKED PacketHello {
		PacketType type;
		SockAddrIn peer;
		SockAddrIn target;
		uint8_t unknown[4];
	};

	struct PACKED PacketPunch {
		PacketType type;
		SockAddrIn addr;
		uint8_t unknown[4];
	};

	struct PACKED PacketChain {
		PacketType type;
		uint32_t spectatorCount;
	};

	struct PACKED PacketInitRequ {
		PacketType type;
		uint8_t gameId[16];
		uint8_t unknown[8];
		RequestType reqType;
		uint8_t nameLength;
		char name[1];
	};

	struct PACKED CharacterSelectKeys {
		bool up: 1;
		bool down: 1;
		bool left: 1;
		bool right: 1;
		bool Z: 1;
		bool X: 1;
		bool C: 1;
		bool Q: 1;
		bool dash: 1;
		bool A: 1;
		unsigned char padding: 6;
	};

	struct PACKED BattleKeys {
		bool up: 1;
		bool down: 1;
		bool left: 1;
		bool right: 1;
		bool A: 1;
		bool B: 1;
		bool C: 1;
		bool dash: 1;
		bool AandB: 1;
		bool BandC: 1;
		unsigned char padding: 6;
	};

	union Inputs {
		CharacterSelectKeys charSelect;
		BattleKeys battle;
	};

	struct PACKED PacketInitSuccAdd {
		PacketType type;
		uint8_t unknown1[8];
		uint8_t dataSize;
		uint8_t unknown2[3];
	};

	struct PACKED PacketInitSucc {
		PacketType type;
		uint8_t unknown1[8];
		uint8_t dataSize;
		uint8_t unknown2[3];
		char hostProfileName[32];
		char clientProfileName[32];
		uint32_t swrDisabled;
	};

	struct PACKED PacketInitError {
		PacketType type;
		InitErrors reason : 32;
	};

	struct PACKED PacketRedirect {
		PacketType type;
		uint32_t childId;
		SockAddrIn target;
		uint8_t unknown[48];
	};

	struct PACKED GameLoadedEvent {
		GameType type;
		SceneId sceneId;
	};

	struct PACKED GameInputEvent {
		GameType type;
		uint32_t frameId;
		SceneId sceneId;
		uint8_t inputCount;
		Inputs inputs[0];
	};

	struct PlayerData {
		Character character;
		uint8_t skinId;
		uint8_t deckId;
		std::vector<uint16_t> cards;
		bool disabledSimultaneousButtons;
	};

	struct MatchData {
		PlayerData host;
		PlayerData client;
		uint8_t stage;
		uint8_t musicId;
		uint32_t seed;
		uint8_t matchId;
	};

	struct ReplayData {
		uint32_t frameId;
		uint32_t endFrameId;
		uint8_t matchId;
		std::vector<Inputs> replayInputs;
	};

	struct PACKED PlayerMatchData {
		Character character;
		uint8_t skinId;
		uint8_t deckId;
		uint8_t deckSize;
		uint16_t cards[0];

		uint8_t &disabledSimultaneousButton()
		{
			return *reinterpret_cast<uint8_t *>(&this->cards[this->deckSize]);
		}

		const uint8_t &disabledSimultaneousButton() const
		{
			return *reinterpret_cast<const uint8_t *>(&this->cards[this->deckSize]);
		}

		uint8_t *getEndPtr()
		{
			return reinterpret_cast<uint8_t *>(&this->cards[this->deckSize]) + 1;
		}

		const uint8_t *getEndPtr() const
		{
			return reinterpret_cast<const uint8_t *>(&this->cards[this->deckSize]) + 1;
		}
	};

	struct PACKED GameMatchEvent {
		GameType type;
		PlayerMatchData host;
		uint8_t data[sizeof(PlayerMatchData) + 256 * 2 + 7];

		PlayerMatchData &client()
		{
			return *reinterpret_cast<PlayerMatchData *>(this->host.getEndPtr());
		}

		uint8_t &stageId()
		{
			return *this->client().getEndPtr();
		}

		uint8_t &musicId()
		{
			return this->client().getEndPtr()[1];
		}

		uint32_t &randomSeed()
		{
			return *reinterpret_cast<uint32_t *>(this->client().getEndPtr()[2]);
		}

		uint8_t &matchId()
		{
			return this->client().getEndPtr()[6];
		}

		const PlayerMatchData &client() const
		{
			return *reinterpret_cast<const PlayerMatchData *>(this->host.getEndPtr());
		}

		const uint8_t &stageId() const
		{
			return *this->client().getEndPtr();
		}

		const uint8_t &musicId() const
		{
			return this->client().getEndPtr()[1];
		}

		const uint32_t &randomSeed() const
		{
			return *reinterpret_cast<const uint32_t *>(this->client().getEndPtr()[2]);
		}

		const uint8_t &matchId() const
		{
			return this->client().getEndPtr()[6];
		}
	};

	std::ostream &operator<<(std::ostream &stream, const PlayerMatchData &data);

	struct PACKED GameReplayEvent {
		GameType type;
		uint8_t replaySize;
		uint8_t compressedData[0];
	};

	struct PACKED GameReplayRequestEvent {
		GameType type;
		uint32_t frameId;
		uint8_t matchId;
	};

	union PACKED GameEvent {
		GameType type;
		GameLoadedEvent loaded;
		GameInputEvent input;
		GameMatchEvent match;
		GameReplayEvent replay;
		GameReplayRequestEvent replayRequest;
	};

	struct PACKED PacketGame {
		PacketType type;
		GameEvent event;
	};

	struct PACKED PacketRollTime {
		PacketType type;
		uint32_t sequenceId;
		uint32_t timeStamp;
	};

	struct PACKED PacketRollState {
		PacketType type;
		int32_t frameId;
		int32_t hostX;
		int32_t hostY;
		int32_t clientX;
		int32_t clientY;
		uint8_t stuff[4];
	};

	struct PACKED PacketRollSettings {
		PacketType type;
		uint8_t maxRollback;
		uint8_t delay;
	};

	union PACKED Packet {
		PacketType type;
		PacketHello hello;
		PacketPunch punch;
		PacketChain chain;
		PacketInitRequ initRequest;
		PacketInitSucc initSuccess;
		PacketInitError initError;
		PacketRedirect redirect;
		PacketGame game;
		PacketRollTime rollTime;
		PacketRollState rollState;
		PacketRollSettings rollSettings;
	};

	std::string PacketTypeToString(PacketType);
	std::string RequestTypeToString(RequestType);
	std::string SceneIdToString(SceneId);
	std::string InitErrorsToString(InitErrors);
	std::string CharacterToString(Character);
	std::string GameTypeToString(GameType);
	void displayPacketContent(std::ostream &stream, const Soku::Packet &packet);
}


#endif //SOKUTEST_PACKET_HPP
