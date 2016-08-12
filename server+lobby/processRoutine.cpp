#include "processRoutine.h"

bool BeCompeletedSendPacket(BYTE type, BYTE size)
{
	switch (type)
	{
	case PacketType::SetID: 
		if (size != sizeof(Packet::SetID)) 
			return false; 
		break;
	case PacketType::Notify:
		if (size != sizeof(Packet::Notify))
			return false;
		break;
	case PacketType::Connect:
		if (size != sizeof(Packet::Connect))
			return false;
		break;
	case PacketType::Disconnect:
		if (size != sizeof(Packet::Disconnect))
			return false;
		break;
	case PacketType::PlayerMove:
		if (size != sizeof(Packet::Player::Move))
			return false;
		break;
	case PacketType::PlayerLight:
		if (size != sizeof(Packet::Player::Light))
			return false; 
		break;
	case PacketType::PlayerShout:
		if (size != sizeof(Packet::Player::Shout))
			return false;
		break;
	case PacketType::PlayerGetItem:
		if (size != sizeof(Packet::Player::GetItem))
			return false;
		break;
	case PacketType::MonsterSetInfo:
		if (size != sizeof(Packet::Monster::SetInfo))
			return false;
		break;
	case PacketType::MonsterMove:
		if (size != sizeof(Packet::Monster::Move))
			return false;
		break;
	case PacketType::MonsterSetPatrolPos:
		if (size != sizeof(Packet::Monster::SetPatrolPos))
			return false;
		break;
	default:
		break;
	}

	return true;
}

void ProcessPacket(int key, unsigned char *packet)
{
	static int count = 0;
	unsigned char packetType = packet[1];

	std::string debugText = "";

	switch (packetType)
	{
	case (BYTE)PacketType::Notify:
		OnReceivePacket::Notify(key, packet);
		break;
	case (BYTE)PacketType::Connect:
		// Connect 동기화 하는 Packet
//		debugText = std::to_string(key) + " ProcessPacket::Connect::Called!!";
		OnReceivePacket::Connect(key, packet);
		SendMonsterSetInfoPacket(key);
		break;
	case (BYTE)PacketType::PlayerMove:
//		debugText = std::to_string(key) + " ProcessPacket::PlayerMove::Called!!";
		OnReceivePacket::PlayerMove(key, packet);
		break;
	case (BYTE)PacketType::PlayerLight:
//		debugText = std::to_string(key) + " ProcessPacket::PlayerLight::Called!!";
		OnReceivePacket::PlayerLight(key, packet);
		break;
	case (BYTE)PacketType::PlayerShout:
//		debugText = std::to_string(key) + " ProcessPacket::PlayerShout::Called!!";
		OnReceivePacket::PlayerShout(key, packet);
		break;
	case (BYTE)PacketType::PlayerGetItem:
//		debugText = std::to_string(key) + " ProcessPacket::PlayerGetItem::Called!!";
		OnReceivePacket::PlayerGetItem(key, packet);
		break;
	case (BYTE)PacketType::MonsterMove:
//		debugText = std::to_string(key) + " ProcessPacket::MonsterMove::Called!!";
		OnReceivePacket::MonsterMove(key, packet);
		break;
	default:
		debugText = "ProcessPacket :: Unknown Packet Type Detected from. " + std::to_string(key);
		DisplayDebugText(debugText);
		return;
	}

	return;
}

void OnReceivePacket::Notify(int key, unsigned char* packet)
{
	std::string debugText = "";
	Packet::Notify *data = reinterpret_cast<Packet::Notify*>(packet);
	int id = data->id;
	int notice = data->notice;

	debugText = "ProcessPacket :: OnReceive Notify Packet :: Send " + std::to_string(data->notice) + " from. " + std::to_string(data->id);
	DisplayDebugText(debugText);
	return;
}

void OnReceivePacket::Connect(int key, unsigned char* packet)
{
	std::string debugText = "";

	Packet::Connect *data = reinterpret_cast<Packet::Connect*>(packet);
	Player transmittedPlayerData;
	transmittedPlayerData.pos.x = data->posX;
	transmittedPlayerData.pos.y = data->posY;
	transmittedPlayerData.pos.z = data->posZ;

	gClientInfoSet->Update(key, transmittedPlayerData);

	// 다른 클라이언트에게 새로운 클라이언트 알림
	
	BroadcastingExceptIndex(key, packet);

	// 새로 들어온 클라이언트에게 이미 들어와있던 플레이어 정보들을 넘김
	Packet::Connect preOtherPlayerPacket;
	preOtherPlayerPacket.size = sizeof(Packet::Connect);
	preOtherPlayerPacket.type = (BYTE)PacketType::Connect;
	
	bool result = false, marked = false;
	LFNode *pred, *curr;
	LFNode *tail = gClientInfoSet->GetTail();
	pred = gClientInfoSet->GetHead();
	curr = pred;
	while (curr->GetNext() != tail)
	{
		curr = pred->GetNextWithMark(&marked);
		if (marked) continue;

		result = gClientInfoSet->Contains(curr->data.id);
		if (result) {
			if (curr->data.isConnect && curr->data.id != key)
			{
				preOtherPlayerPacket.id = curr->data.id;
				preOtherPlayerPacket.posX = curr->data.player.pos.x;
				preOtherPlayerPacket.posY = curr->data.player.pos.y;
				preOtherPlayerPacket.posZ = curr->data.player.pos.z;
				SendPacket(key, reinterpret_cast<unsigned char*>(&preOtherPlayerPacket));

				debugText = "ProcessPacket :: OnReceive Connect Packet :: Send " + std::to_string(curr->data.id) + " client data To. " + std::to_string(key);
				DisplayDebugText(debugText);
			}
			pred = curr;
		}
	}
	return;
}

void OnReceivePacket::PlayerMove(int key, unsigned char* packet)
{
	int id = 0;
	Player transmittedPlayerData;

	Packet::Player::Move *data = reinterpret_cast<Packet::Player::Move*>(packet);

	id = data->id;
	transmittedPlayerData.pos.x = data->posX;
	transmittedPlayerData.pos.y = data->posY;
	transmittedPlayerData.pos.z = data->posZ;

	gClientInfoSet->Update(id, transmittedPlayerData);
	
	std::string debugText = "ProcessPacket :: OnReceive PlayerMove Packet :: " + std::to_string(id) +" client Pos("
		+ std::to_string(transmittedPlayerData.pos.x) + ", " + std::to_string(transmittedPlayerData.pos.y) + ", " + std::to_string(transmittedPlayerData.pos.z) + ")";
	DisplayDebugText(debugText);

	BroadcastingExceptIndex(key, packet);

	return;
}

void OnReceivePacket::PlayerLight(int key, unsigned char* packet)
{
	BroadcastingExceptIndex(key, packet);
	return;
}

void OnReceivePacket::PlayerShout(int key, unsigned char* packet)
{
	BroadcastingExceptIndex(key, packet);
	return;
}

void OnReceivePacket::PlayerGetItem(int key, unsigned char* packet)
{
	Packet::Player::GetItem *data = reinterpret_cast<Packet::Player::GetItem*>(packet);

	std::string debugText = "ProcessPacket :: OnReceive PlayerGetItem Packet:: " + std::to_string(data->id) + " get item " + std::to_string(data->itemID);
	DisplayDebugText(debugText);

	BroadcastingExceptIndex(key, packet);
	return;
}

void OnReceivePacket::MonsterMove(int key, unsigned char *packet)
{
	bool retval = true;

	Packet::Monster::Move *data = reinterpret_cast<Packet::Monster::Move*>(packet);

	gLock.lock();
	retval &= (abs(gMonster.pos.x - data->posX) < 2.0f) ? true : false;
	retval &= (abs(gMonster.pos.y - data->posY) < 2.0f) ? true : false;
	retval &= (abs(gMonster.pos.z - data->posZ) < 2.0f) ? true : false;

	// 변화가 작다면 그냥 종료합니다.
	if (retval) {
		gLock.unlock();
		return;
	}

	// 변화가 크다면 그 내용을 저장합니다.
	gMonster.pos.x = data->posX;
	gMonster.pos.y = data->posY;
	gMonster.pos.z = data->posZ;
	
	std::string debugText = "ProcessPacket :: OnReceive MonsterMove Packet :: Current Monster Pos (" 
		+ std::to_string(gMonster.pos.x) + ", " + std::to_string(gMonster.pos.y) + ", " + std::to_string(gMonster.pos.z) + ")";
	gLock.unlock();
	//DisplayDebugText(debugText);
	

	if (HasMonsterArrivedAtDestination())
	{
		SetMonsterNewPatrolPath();
		DisplayDebugText("True");
		/*
		gLock.lock();
		
		std::string debugText = "OnReceiveMonsterMovePacket::gMonsterPatrolPos ("
			+ std::to_string(gMonster.patrolPos.x) + ", " + std::to_string(gMonster.patrolPos.y) + ", " + std::to_string(gMonster.patrolPos.z) + ")";
		gLock.unlock();
		
		DisplayDebugText(debugText);
		*/
		SendMonsterSetPatrolPosPacket();
	}
	return;
}

void InitializeMonster(void)
{
	monsterPath[0] = { 160.0f,	0.0f,	30.0f	};
	monsterPath[1] = { 90.0f,	0.0f,	-60.0f	}; 
	monsterPath[2] = { 160.0f,	0.0f,	-80.0f	};
	monsterPath[3] = { 200.0f,	0.0f,	0.0f	};

	gLock.lock();
	gMonster.pos = { 160, 0, -83 };
	gLock.unlock();

	SetMonsterNewPatrolPath();
	return;
}

void SendMonsterSetInfoPacket(int key)
{
	// 새로 들어온 클라이언트에게 몬스터 정보 동기화
	Packet::Monster::SetInfo monsterSetInfoPacket;
	monsterSetInfoPacket.size = sizeof(Packet::Monster::SetInfo);
	monsterSetInfoPacket.type = (BYTE)PacketType::MonsterSetInfo;
	gLock.lock();
	monsterSetInfoPacket.posX = gMonster.pos.x;
	monsterSetInfoPacket.posY = gMonster.pos.y;
	monsterSetInfoPacket.posZ = gMonster.pos.z;
	monsterSetInfoPacket.patrolPosX = gMonster.patrolPos.x;
	monsterSetInfoPacket.patrolPosY = gMonster.patrolPos.y;
	monsterSetInfoPacket.patrolPosZ = gMonster.patrolPos.z;
	gLock.unlock();

	SendPacket(key, reinterpret_cast<unsigned char*>(&monsterSetInfoPacket));

	std::string debugText = "";
	gLock.lock();
	debugText = "ProcessPacket :: Send MonsterSetIntfo Packet :: Send Monster Pos ("
		+ std::to_string((int)gMonster.pos.x) + ", " + std::to_string((int)gMonster.pos.y) + ", " + std::to_string((int)gMonster.pos.z) + ") Data To. " + std::to_string(key);
	gLock.unlock();
	DisplayDebugText(debugText);

	return;
}

bool HasMonsterArrivedAtDestination(void)
{
	bool ret = true;
	gLock.lock();
	
	ret &= (abs(gMonster.pos.x - gMonster.patrolPos.x) <= 3.0f) ? true : false;
	ret &= (abs(gMonster.pos.y - gMonster.patrolPos.y) <= 3.0f) ? true : false;
	ret &= (abs(gMonster.pos.z - gMonster.patrolPos.z) <= 3.0f) ? true : false;

	gLock.unlock();

	return ret;
}

void SetMonsterNewPatrolPath(void)
{
	// rand 함수 변경 필요
	int randVal = 0;
	bool done = false;

	while (!done) {
		bool same = true;
		randVal = rand() % NUM_OF_MONSTER_PATH;
		gLock.lock();
		same &= (gMonster.patrolPos.x == monsterPath[randVal].x) ? true : false;
		same &= (gMonster.patrolPos.y == monsterPath[randVal].y) ? true : false;
		same &= (gMonster.patrolPos.z == monsterPath[randVal].z) ? true : false;
		gLock.unlock();

		if (!same)
		{
			gLock.lock();
			gMonster.patrolPos = monsterPath[randVal];
			gLock.unlock();

			done = true;
		}
	}

	return;
}

void SendMonsterSetPatrolPosPacket(void)
{
	Packet::Monster::SetPatrolPos packet;
	packet.size = sizeof(Packet::Monster::SetPatrolPos);
	packet.type = (BYTE)PacketType::MonsterSetPatrolPos;
	gLock.lock();
	packet.posX = gMonster.patrolPos.x;
	packet.posY = gMonster.patrolPos.y;
	packet.posZ = gMonster.patrolPos.z;
	gLock.unlock();

	Broadcasting(reinterpret_cast<unsigned char*>(&packet));

	return;
}