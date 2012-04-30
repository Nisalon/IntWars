#include "PacketHandler.h"

bool PacketHandler::handleNull(HANDLE_ARGS)
{
	return true;
}

bool PacketHandler::handleKeyCheck(ENetPeer *peer, ENetPacket *packet)
{
	KeyCheck *keyCheck = (KeyCheck*)packet->data;
	uint64 userId = _blowfish->Decrypt(keyCheck->checkId);

	if(userId == keyCheck->userId)
	{
		PDEBUG_LOG_LINE(Log::getMainInstance()," User got the same key as i do, go on!\n");
		peerInfo(peer)->keyChecked = true;
		peerInfo(peer)->userId = userId;
	}
	else
	{
		Log::getMainInstance()->errorLine(" WRONG KEY, GTFO!!!\n");
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
	Log::getMainInstance()->writeLine("Client version: %s\n", version->version);

	SynchVersionAns answer;
	answer.mapId = 1;
	answer.players[0].userId = peerInfo(peer)->userId;
	answer.players[0].skill1 = SPL_Exhaust;
	answer.players[0].skill2 = SPL_Cleanse;

	return sendPacket(peer, reinterpret_cast<uint8*>(&answer), sizeof(SynchVersionAns), 3);
}

bool PacketHandler::handleMap(ENetPeer *peer, ENetPacket *packet)
{
	LoadScreenPlayer *playerName = LoadScreenPlayer::create(PKT_S2C_LoadName, peerInfo(peer)->name, peerInfo(peer)->nameLen);
	playerName->userId = peerInfo(peer)->userId;

	LoadScreenPlayer *playerHero = LoadScreenPlayer::create(PKT_S2C_LoadHero,  peerInfo(peer)->type, peerInfo(peer)->typeLen);
	playerHero->userId = peerInfo(peer)->userId;
	playerHero->skinId = 2;

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
	spawn.netId = 0x40000019; 
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
	return true;
}

bool PacketHandler::handleAttentionPing(ENetPeer *peer, ENetPacket *packet)
{
	AttentionPing *ping = reinterpret_cast<AttentionPing*>(packet->data);
	AttentionPingAns response(ping);

	Log::getMainInstance()->writeLine("Plong x: %f, y: %f, z: %f, type: %i\n", ping->x, ping->y, ping->z, ping->type);

	return broadcastPacket(reinterpret_cast<uint8*>(&response), sizeof(AttentionPing), 3);
}

bool PacketHandler::handleView(ENetPeer *peer, ENetPacket *packet)
{
	ViewReq *request = reinterpret_cast<ViewReq*>(packet->data);

	Log::getMainInstance()->writeLine("View (%i), x:%f, y:%f, zoom: %f\n", request->requestNo, request->x, request->y, request->zoom);

	
	ViewAns answer;
	answer.requestNo = request->requestNo;

	if(request->requestNo == 0)
	{
		sendPacket(peer, reinterpret_cast<uint8*>(&answer), sizeof(ViewAns), 3, UNRELIABLE);
		answer.requestNo = 1;
		return sendPacket(peer, reinterpret_cast<uint8*>(&answer), sizeof(ViewAns), 3, UNRELIABLE);
	}else
		return sendPacket(peer, reinterpret_cast<uint8*>(&answer), sizeof(ViewAns), 3, UNRELIABLE);
}

bool PacketHandler::handleMove(ENetPeer *peer, ENetPacket *packet)
{
	MovementReq *request = reinterpret_cast<MovementReq*>(packet->data);

	Log::getMainInstance()->writeLine("Move to: x:%f, y:%f, z: %f, unk: %i, vectorNo: %i\n", request->x, request->y, request->z, request->unk, request->vectorNo);
	for(int i = 0; i < request->vectorNo; i++)
		printf("     Vector %i, x: %i, y: %i\n", i, request->getVector(i)->x, request->getVector(i)->y);

	MovementAns *answer = MovementAns::create(request->vectorNo);
	answer->header.ticks = peerInfo(peer)->getTicks();
	answer->ok = 1;
	answer->vectorNo = request->vectorNo;
	answer->netId = peerInfo(peer)->netId;
	for(int i = 0; i < request->vectorNo; i++)
	{
		answer->getVector(i)->x = request->getVector(i)->x;
		answer->getVector(i)->y = request->getVector(i)->y;
	}
	//Log::getMainInstance()->writeLine("Move to: x(left->right):%f, y(height):%f, z(bot->top): %f\n", request->x1, request->y1, request->z1);
	return broadcastPacket(reinterpret_cast<uint8*>(answer), answer->size(), 4);
	return true;
}

bool PacketHandler::handleLoadPing(ENetPeer *peer, ENetPacket *packet)
{
	PingLoadInfo *loadInfo = reinterpret_cast<PingLoadInfo*>(packet->data);

	PingLoadInfo response;
	memcpy(&response, packet->data, sizeof(PingLoadInfo));
	response.header.cmd = PKT_S2C_Ping_Load_Info;
	response.userId = peerInfo(peer)->userId;


	Log::getMainInstance()->writeLine("loaded: %f, ping: %f, %i, %f\n", loadInfo->loaded, loadInfo->ping, loadInfo->unk4, loadInfo->f3);
	return broadcastPacket(reinterpret_cast<uint8*>(&response), sizeof(PingLoadInfo), 4, UNRELIABLE);
}

bool PacketHandler::handleQueryStatus(HANDLE_ARGS)
{
	QueryStatus response;
	return sendPacket(peer, reinterpret_cast<uint8*>(&response), sizeof(QueryStatus), 3);
}

bool PacketHandler::handleChatBoxMessage(HANDLE_ARGS)
{
	ChatBoxMessage* message = reinterpret_cast<ChatBoxMessage*>(packet->data);

	switch(message->cmd)
	{
	case CMT_ALL:
	//!TODO make a player class and foreach player in game send the message
		return sendPacket(peer,packet->data,packet->dataLength,CHL_COMMUNICATION);
		break;
	case CMT_TEAM:
	//!TODO make a team class and foreach player in the team send the message
		return sendPacket(peer,packet->data,packet->dataLength,CHL_COMMUNICATION);
		break;
	default:
		Log::getMainInstance()->errorLine("Unknow ChatMessageType\n");
		return sendPacket(peer,packet->data,packet->dataLength,CHL_COMMUNICATION);
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

	FogUpdate2 test;
	test.x = 0;
	test.y = 0;
	test.radius = 1;
	test.unk1 = 2;
	//uint8 p[] = {0xC5, 0x19, 0x00, 0x00, 0x40, 0x00, 0x00, 0x50};
	//sendPacket(peer, reinterpret_cast<uint8*>(p), sizeof(p), 3);
	sendPacket(peer, reinterpret_cast<uint8*>(&test), sizeof(FogUpdate2), 3);
	
	return sendPacket(peer, reinterpret_cast<uint8*>(&skillUpResponse),sizeof(skillUpResponse),CHL_GAMEPLAY);

}