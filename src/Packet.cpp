//
// Created by Gegel85 on 04/06/2020.
//

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>
#include "Packet.hpp"

namespace Soku
{
	uint8_t Soku110acRollSWRAllChars[16] = {
		0x64, 0x73, 0x65, 0xD9,
		0xFF, 0xC4, 0x6E, 0x48,
		0x8D, 0x7C, 0xA1, 0x92,
		0x31, 0x34, 0x72, 0x95
	};
	uint8_t Soku110acNoRollSWRAllChars[16] = {
		0x6E, 0x73, 0x65, 0xD9,
		0xFF, 0xC4, 0x6E, 0x48,
		0x8D, 0x7C, 0xA1, 0x92,
		0x31, 0x34, 0x72, 0x95
	};
	uint8_t Soku110acNoSWRAllChars[16] = {
		0x46, 0xC9, 0x67, 0xC8,
		0xAC, 0xF2, 0x44, 0x4D,
		0xB8, 0xB1, 0xEC, 0xEE,
		0xD4, 0xD5, 0x40, 0x4A
	};

	void displayGameEvent(std::ostream &stream, const Soku::GameEvent &event)
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
			stream << ", stageId: " << event.match.stageId();
			stream << ", musicId: " << event.match.musicId();
			stream << ", randomSeed: " << event.match.randomSeed();
			stream << ", matchId: " << event.match.matchId();
			break;
		case Soku::GAME_REPLAY:
			break;
		case Soku::GAME_REPLAY_REQUEST:
			stream << ", frameId" << event.replayRequest.frameId;
			stream << ", matchId" << event.replayRequest.matchId;
			break;
		case Soku::GAME_MATCH_ACK:
		case Soku::GAME_MATCH_REQUEST:
			break;
		}
	}

	std::ostream &operator<<(std::ostream &stream, const PlayerMatchData &data)
	{
		stream << "{character: " << CharacterToString(data.character);
		stream << ", skinId: " << data.skinId;
		stream << ", deckId: " << data.deckId;
		stream << ", deckSize: " << data.deckSize;
		stream << ", cards: [" << std::hex;
		for (int i = 0; i < data.deckSize; i++)
			stream << (i == 0 ? "" : ", ") << data.cards[i];
		stream << "]";
		stream << ", disabledSimultaneousButton: " << std::boolalpha << data.disabledSimultaneousButton() << std::noboolalpha << "}";
		return stream;
	}

	void displayPacketContent(std::ostream &stream, const Soku::Packet &packet)
	{
		stream << "type: " << PacketTypeToString(packet.type);
		switch (packet.type) {
		case Soku::HELLO:
			stream << ", peer: ";
			stream << static_cast<int>(packet.hello.peer.sin_addr.S_un.S_un_b.s_b1) << ".";
			stream << static_cast<int>(packet.hello.peer.sin_addr.S_un.S_un_b.s_b2) << ".";
			stream << static_cast<int>(packet.hello.peer.sin_addr.S_un.S_un_b.s_b3) << ".";
			stream << static_cast<int>(packet.hello.peer.sin_addr.S_un.S_un_b.s_b4) << ":";
			stream << static_cast<int>(htons(packet.hello.peer.sin_port)) << ", target: ";
			stream << static_cast<int>(packet.hello.target.sin_addr.S_un.S_un_b.s_b1) << ".";
			stream << static_cast<int>(packet.hello.target.sin_addr.S_un.S_un_b.s_b2) << ".";
			stream << static_cast<int>(packet.hello.target.sin_addr.S_un.S_un_b.s_b3) << ".";
			stream << static_cast<int>(packet.hello.target.sin_addr.S_un.S_un_b.s_b4) << ":";
			stream << static_cast<int>(htons(packet.hello.target.sin_port)) << ", unknown: [";
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

	std::string PacketTypeToString(PacketType e)
	{
		switch (e) {
		case HELLO:
			return "HELLO";
		case PUNCH:
			return "PUNCH";
		case OLLEH:
			return "OLLEH";
		case CHAIN:
			return "CHAIN";
		case INIT_REQUEST:
			return "INIT_REQUEST";
		case INIT_SUCCESS:
			return "INIT_SUCCESS";
		case INIT_ERROR:
			return "INIT_ERROR";
		case REDIRECT:
			return "REDIRECT";
		case QUIT:
			return "QUIT";
		case HOST_GAME:
			return "HOST_GAME";
		case CLIENT_GAME:
			return "CLIENT_GAME";
		case SOKUROLL_TIME:
			return "SOKUROLL_TIME";
		case SOKUROLL_TIME_ACK:
			return "SOKUROLL_TIME_ACK";
		case SOKUROLL_STATE:
			return "SOKUROLL_STATE";
		case SOKUROLL_SETTINGS:
			return "SOKUROLL_SETTINGS";
		case SOKUROLL_SETTINGS_ACK:
			return "SOKUROLL_SETTINGS_ACK";
		default:
			return "Unknown PacketType " + std::to_string(e);
		}
	}

	std::string RequestTypeToString(RequestType e)
	{
		switch (e) {
		case SPECTATE_REQU:
			return "SPECTATE_REQU";
		case PLAY_REQU:
			return "PLAY_REQU";
		default:
			return "Unknown RequestType " + std::to_string(e);
		}
	}

	std::string SceneIdToString(SceneId e)
	{
		switch (e) {
		case SCENE_CHARACTER_SELECT:
			return "SCENE_CHARACTER_SELECT";
		case SCENE_BATTLE:
			return "SCENE_BATTLE";
		default:
			return "Unknown SceneId " + std::to_string(e);
		}
	}

	std::string InitErrorsToString(InitErrors e)
	{
		switch (e) {
		case ERROR_SPECTATE_DISABLED:
			return "ERROR_SPECTATE_DISABLED";
		case ERROR_GAME_STATE_INVALID:
			return "ERROR_GAME_STATE_INVALID";
		default:
			return "Unknown InitErrors " + std::to_string(e);
		}
	}

	std::string CharacterToString(Character e)
	{
		switch (e) {
		case CHARACTER_REIMU:
			return "REIMU";
		case CHARACTER_MARISA:
			return "MARISA";
		case CHARACTER_SAKUYA:
			return "SAKUYA";
		case CHARACTER_ALICE:
			return "ALICE";
		case CHARACTER_PATCHOULI:
			return "PATCHOULI";
		case CHARACTER_YOUMU:
			return "YOUMU";
		case CHARACTER_REMILIA:
			return "REMILIA";
		case CHARACTER_YUYUKO:
			return "YUYUKO";
		case CHARACTER_YUKARI:
			return "YUKARI";
		case CHARACTER_SUIKA:
			return "SUIKA";
		case CHARACTER_REISEN:
			return "REISEN";
		case CHARACTER_AYA:
			return "AYA";
		case CHARACTER_KOMACHI:
			return "KOMACHI";
		case CHARACTER_IKU:
			return "IKU";
		case CHARACTER_TENSHI:
			return "TENSHI";
		case CHARACTER_SANAE:
			return "SANAE";
		case CHARACTER_CIRNO:
			return "CIRNO";
		case CHARACTER_MEILING:
			return "MEILING";
		case CHARACTER_UTSUHO:
			return "UTSUHO";
		case CHARACTER_SUWAKO:
			return "SUWAKO";
		default:
			return "Unknown Character " + std::to_string(e);
		}
	}

	std::string GameTypeToString(GameType e)
	{
		switch (e) {
		case GAME_LOADED:
			return "GAME_LOADED";
		case GAME_LOADED_ACK:
			return "GAME_LOADED_ACK";
		case GAME_INPUT:
			return "GAME_INPUT";
		case GAME_MATCH:
			return "GAME_MATCH";
		case GAME_MATCH_ACK:
			return "GAME_MATCH_ACK";
		case GAME_MATCH_REQUEST:
			return "GAME_MATCH_REQUEST";
		case GAME_REPLAY:
			return "GAME_REPLAY";
		case GAME_REPLAY_REQUEST:
			return "GAME_REPLAY_REQUEST";
		default:
			return "Unknown GameType " + std::to_string(e);
		}
	}
}