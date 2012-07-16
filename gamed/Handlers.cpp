/*
IntWars playground server for League of Legends protocol testing
Copyright (C) 2012  Intline9 <Intline9@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"
#include "PacketHandler.h"

bool PacketHandler::handleNull(HANDLE_ARGS)
{
	return true;
}

bool PacketHandler::handleKeyCheck(ENetPeer *peer, ENetPacket *packet)
{
	KeyCheck *keyCheck = (KeyCheck*)packet->data;
	uint64 userId = _blowfish->Decrypt(keyCheck->checkId);

/*
	uint64 enc = _blowfish->Encrypt(keyCheck->userId);
	char buffer[255];
	unsigned char *p = (unsigned char*)&enc;
	for(int i = 0; i < 8; i++)
	{
		sprintf(&buffer[i*3], "%02X ", p[i]);
	}
	PDEBUG_LOG_LINE(Logging," Enc id: %s\n", buffer);*/
	
	if(userId == keyCheck->userId)
	{
		PDEBUG_LOG_LINE(Logging," User got the same key as i do, go on!\n");
		peerInfo(peer)->keyChecked = true;
		peerInfo(peer)->userId = userId;
	}
	else
	{
		Logging->errorLine(" WRONG KEY, GTFO!!!\n");
		return false;
	}

	//Send response as this is correct (OFC DO SOME ID CHECKS HERE!!!)
	KeyCheck response;
	response.userId = keyCheck->userId;
	
	return sendPacket(peer, reinterpret_cast<uint8*>(&response), sizeof(KeyCheck), CHL_HANDSHAKE);
}

bool PacketHandler::handleGameNumber(ENetPeer *peer, ENetPacket *packet)
{
	WorldSendGameNumber world;
	world.gameId = 1;
	memcpy(world.data, peerInfo(peer)->name, peerInfo(peer)->nameLen);

	return sendPacket(peer, reinterpret_cast<uint8*>(&world), sizeof(WorldSendGameNumber), CHL_S2C);
}

bool PacketHandler::handleSynch(ENetPeer *peer, ENetPacket *packet)
{
	SynchVersion *version = reinterpret_cast<SynchVersion*>(packet->data);
	Logging->writeLine("Client version: %s\n", version->version);

	SynchVersionAns answer;
	answer.mapId = 1;
	answer.players[0].userId = peerInfo(peer)->userId;
	answer.players[0].skill1 = SPL_Ignite;
	answer.players[0].skill2 = SPL_Flash;

	return sendPacket(peer, reinterpret_cast<uint8*>(&answer), sizeof(SynchVersionAns), 3);
}

bool PacketHandler::handleMap(ENetPeer *peer, ENetPacket *packet)
{
	LoadScreenPlayer *playerName = LoadScreenPlayer::create(PKT_S2C_LoadName, peerInfo(peer)->name, peerInfo(peer)->nameLen);
	playerName->userId = peerInfo(peer)->userId;

	LoadScreenPlayer *playerHero = LoadScreenPlayer::create(PKT_S2C_LoadHero,  peerInfo(peer)->type, peerInfo(peer)->typeLen);
	playerHero->userId = peerInfo(peer)->userId;
	playerHero->skinId = peerInfo(peer)->skinNo;

	//Builds team info
	LoadScreenInfo screenInfo;
	screenInfo.bluePlayerNo = 1;
	screenInfo.redPlayerNo = 0;

	screenInfo.bluePlayerIds[0] = peerInfo(peer)->userId;

	bool pInfo = sendPacket(peer, reinterpret_cast<uint8*>(&screenInfo), sizeof(LoadScreenInfo), CHL_LOADING_SCREEN);

	//For all players send this info
	bool pName = sendPacket(peer, reinterpret_cast<uint8*>(playerName), playerName->getPacketLength(), CHL_LOADING_SCREEN);
	bool pHero = sendPacket(peer, reinterpret_cast<uint8*>(playerHero), playerHero->getPacketLength(), CHL_LOADING_SCREEN);

	//cleanup
	LoadScreenPlayer::destroy(playerName);
	LoadScreenPlayer::destroy(playerHero);

	return (pInfo && pName && pHero);

}

//building the map
bool PacketHandler::handleSpawn(ENetPeer *peer, ENetPacket *packet)
{
	HeroSpawn spawn;
	spawn.netId = peerInfo(peer)->netId; 
	spawn.gameId = 0;
	memcpy(spawn.name, peerInfo(peer)->name, peerInfo(peer)->nameLen);
	memcpy(spawn.type, peerInfo(peer)->type, peerInfo(peer)->typeLen);

	StatePacket end(PKT_S2C_EndSpawn);

	bool p1 = sendPacket(peer, reinterpret_cast<uint8*>(&spawn), sizeof(HeroSpawn), CHL_S2C);
	bool p2 = sendPacket(peer, reinterpret_cast<uint8*>(&end), sizeof(StatePacket), CHL_S2C);
	return p1 & p2;
}

bool PacketHandler::handleStartGame(HANDLE_ARGS)
{
	StatePacket start(PKT_S2C_StartGame);
	sendPacket(peer, reinterpret_cast<uint8*>(&start), sizeof(StatePacket), CHL_S2C);
	FogUpdate2 test;
	test.x = 0;
	test.y = 0;
	test.radius = 1;
	test.unk1 = 2;
	//uint8 p[] = {0xC5, 0x19, 0x00, 0x00, 0x40, 0x00, 0x00, 0x50};
	//sendPacket(peer, reinterpret_cast<uint8*>(p), sizeof(p), 3);
	sendPacket(peer, reinterpret_cast<uint8*>(&test), sizeof(FogUpdate2), 3);

	//playing around 8-)
/*
	CharacterStats movement;
	movement.netId = peerInfo(peer)->netId;
	movement.statType = STI_Movement;
	movement.statValue = 800;
	sendPacket(peer,reinterpret_cast<uint8*>(&movement),sizeof(movement), 4);*/

	return true;
}

bool PacketHandler::handleAttentionPing(ENetPeer *peer, ENetPacket *packet)
{
	AttentionPing *ping = reinterpret_cast<AttentionPing*>(packet->data);
	AttentionPingAns response(ping);

	Logging->writeLine("Plong x: %f, y: %f, z: %f, type: %i\n", ping->x, ping->y, ping->z, ping->type);

	return broadcastPacket(reinterpret_cast<uint8*>(&response), sizeof(AttentionPing), 3);
}

bool PacketHandler::handleView(ENetPeer *peer, ENetPacket *packet)
{
	ViewReq *request = reinterpret_cast<ViewReq*>(packet->data);

	Logging->writeLine("View (%i), x:%f, y:%f, zoom: %f\n", request->requestNo, request->x, request->y, request->zoom);

	ViewAns answer;
	answer.requestNo = request->requestNo;

	sendPacket(peer, reinterpret_cast<uint8*>(&answer), sizeof(ViewAns), CHL_S2C, UNRELIABLE);
	enet_host_flush(peer->host);
	
	if(request->requestNo == 0xFE)
	{
		answer.requestNo = 0xFF;
		sendPacket(peer, reinterpret_cast<uint8*>(&answer), sizeof(ViewAns), CHL_S2C, UNRELIABLE);
	}

	return true;

}

bool PacketHandler::handleMove(ENetPeer *peer, ENetPacket *packet)
{
	MovementReq *request = reinterpret_cast<MovementReq*>(packet->data);

	switch(request->type)
	{
		//TODO, Implement stop commands
		case STOP:
			Logging->writeLine("Move stop\n");
			return true;
		case EMOTE:
			Logging->writeLine("Emotion\n");
			return true;
	}

	Logging->writeLine("Move to(normal): x:%f, y:%f, z: %f, type: %i, vectorNo: %i\n", request->x, request->y, request->z, request->type, request->vectorNo);
	for(int i = 0; i < request->vectorNo; i++)
		printf("     Vector %i, x: %i, y: %i\n", i, request->getVector(i)->x, request->getVector(i)->y);

	MovementAns *answer = MovementAns::create(request->vectorNo, request->hasDelta());
	answer->header.ticks = peerInfo(peer)->getTicks();
	answer->ok = 1;
	answer->vectorNo = request->vectorNo;
	answer->netId = peerInfo(peer)->netId;
	for(int i = 0; i < request->vectorNo; i++)
	{
		answer->getVector(i)->x = request->getVector(i)->x;
		answer->getVector(i)->y = request->getVector(i)->y;
	}

	return broadcastPacket(reinterpret_cast<uint8*>(answer), answer->size(), 4);
}

bool PacketHandler::handleLoadPing(ENetPeer *peer, ENetPacket *packet)
{
	PingLoadInfo *loadInfo = reinterpret_cast<PingLoadInfo*>(packet->data);

	PingLoadInfo response;
	memcpy(&response, packet->data, sizeof(PingLoadInfo));
	response.header.cmd = PKT_S2C_Ping_Load_Info;
	response.userId = peerInfo(peer)->userId;


	Logging->writeLine("loaded: %f, ping: %f, %i, %f\n", loadInfo->loaded, loadInfo->ping, loadInfo->unk4, loadInfo->f3);
	return broadcastPacket(reinterpret_cast<uint8*>(&response), sizeof(PingLoadInfo), 4, UNRELIABLE);
}

bool PacketHandler::handleQueryStatus(HANDLE_ARGS)
{
	QueryStatus response;
	return sendPacket(peer, reinterpret_cast<uint8*>(&response), sizeof(QueryStatus), 3);
}

bool PacketHandler::handleChatBoxMessage(HANDLE_ARGS)
{
	ChatMessage* message = reinterpret_cast<ChatMessage*>(packet->data);

	//Lets do commands
	if(message->msg == '.')
	{
		const char *cmd[] = {".gold", ".speed", ".health", ".xp", ".ap", ".ad", ".mana", ".help"};


		// Set Gold
		if(strncmp(message->getMessage(), cmd[0], strlen(cmd[0])) == 0)
		{
			float gold = (float)atoi(&message->getMessage()[strlen(cmd[0])+1]);
			CharacterStats *stats = CharacterStats::create(FM1_Gold, 0, 0, 0, 0);
			stats->netId = peerInfo(peer)->netId;
			stats->setValue(1, FM1_Gold, gold);
			
			Logging->writeLine("Set gold to %f\n", gold);
			sendPacket(peer, reinterpret_cast<uint8*>(stats), stats->getSize(), CHL_LOW_PRIORITY, 2);
			stats->destroy();
			return true;
		}
		/*

		//movement
		if(strncmp(message->getMessage(), cmd[2], strlen(cmd[2])) == 0)
		{
			float data = (float)atoi(&message->getMessage()[strlen(cmd[2])+1]);

			charStats.statType = STI_Movement;
			charStats.statValue = data;
			Logging->writeLine("set champ speed to %f\n", data);
			sendPacket(peer,reinterpret_cast<uint8*>(&charStats),sizeof(charStats), CHL_LOW_PRIORITY, 2);
			return true;
		}
		//health
		if(strncmp(message->getMessage(), cmd[3], strlen(cmd[3])) == 0)
		{
			float data = (float)atoi(&message->getMessage()[strlen(cmd[3])+1]);

			charStats.statType = STI_Health;
			charStats.statValue = data;
			Logging->writeLine("set champ health to %f\n", data);
			sendPacket(peer,reinterpret_cast<uint8*>(&charStats),sizeof(charStats), CHL_LOW_PRIORITY, 2);
			return true;
		}
		//experience
		if(strncmp(message->getMessage(), cmd[4], strlen(cmd[4])) == 0)
		{
			float data = (float)atoi(&message->getMessage()[strlen(cmd[4])+1]);

			charStats.statType = STI_Exp;
			charStats.statValue = data;
			Logging->writeLine("set champ exp to %f\n", data);
			sendPacket(peer,reinterpret_cast<uint8*>(&charStats),sizeof(charStats), CHL_LOW_PRIORITY, 2);
			return true;
		}
		//AbilityPower
		if(strncmp(message->getMessage(), cmd[5], strlen(cmd[5])) == 0)
		{
			float data = (float)atoi(&message->getMessage()[strlen(cmd[5])+1]);

			charStats.statType = STI_AbilityPower;
			charStats.statValue = data;
			Logging->writeLine("set champ abilityPower to %f\n", data);
			sendPacket(peer,reinterpret_cast<uint8*>(&charStats),sizeof(charStats), CHL_LOW_PRIORITY, 2);
			return true;
		}
		//Attack damage
		if(strncmp(message->getMessage(), cmd[6], strlen(cmd[6])) == 0)
		{
			float data = (float)atoi(&message->getMessage()[strlen(cmd[6])+1]);

			charStats.statType = STI_AttackDamage;
			charStats.statValue = data;
			Logging->writeLine("set champ attack damage to %f\n", data);
			sendPacket(peer,reinterpret_cast<uint8*>(&charStats),sizeof(charStats), CHL_LOW_PRIORITY, 2);
			return true;
		}
		//Mana
		if(strncmp(message->getMessage(), cmd[7], strlen(cmd[7])) == 0)
		{
			float data = (float)atoi(&message->getMessage()[strlen(cmd[7])+1]);

			charStats.statType = STI_Mana;
			charStats.statValue = data;
			Logging->writeLine("set champ mana to %f\n", data);
			sendPacket(peer,reinterpret_cast<uint8*>(&charStats),sizeof(charStats), CHL_LOW_PRIORITY, 2);
			return true;
		}
		*/

	}

	switch(message->type)
	{
	case CMT_ALL:
		return broadcastPacket(packet->data, packet->dataLength, CHL_COMMUNICATION);
		break;
	case CMT_TEAM:
	//!TODO make a team class and foreach player in the team send the message
		return sendPacket(peer, packet->data, packet->dataLength, CHL_COMMUNICATION);
		break;
	default:
		Logging->errorLine("Unknown ChatMessageType\n");
		return sendPacket(peer,packet->data,packet->dataLength, CHL_COMMUNICATION);
		break;
	}
	return false;
}

bool PacketHandler::handleSkillUp(HANDLE_ARGS) {

	SkillUpPacket* skillUpPacket = reinterpret_cast<SkillUpPacket*>(packet->data);
	//!TODO Check if can up skill? :)
	SkillUpResponse skillUpResponse;
	
	skillUpResponse.skill = skillUpPacket->skill;
	skillUpResponse.level = 0x0001;
	
	return sendPacket(peer, reinterpret_cast<uint8*>(&skillUpResponse),sizeof(skillUpResponse),CHL_GAMEPLAY);

}

bool PacketHandler::handleEmotion(HANDLE_ARGS) {
	EmotionPacket* emotion = reinterpret_cast<EmotionPacket*>(packet->data);
	//for later use -> tracking, etc.
	switch(emotion->id)
	{
	case 0:
		//dance
		Logging->writeLine("dance");
		break;
	case 1:
		//taunt
		Logging->writeLine("taunt");
		break;
	case 2:
		//laugh
		Logging->writeLine("laugh");
		break;
	case 3:
		//joke
		Logging->writeLine("joke");
		break;
	}
	EmotionResponse response;
	response.header.netId = emotion->header.netId;
	response.id = emotion->id;

	return broadcastPacket(reinterpret_cast<uint8*>(&response), sizeof(response), CHL_S2C);

}