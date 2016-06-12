#include "processRoutine.h"

bool BeCompeletedSendPacket(BYTE type, BYTE size)
{
	switch (type)
	{
	case PacketType::SetID: 
		if (size != sizeof(Packet::SetID)) 
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
	case (BYTE)PacketType::Connect:
		// Connect ����ȭ �ϴ� Packet
		debugText = std::to_string(key) + " ProcessPacket::Connect::Called!!";
		OnReceivePacket::Connect(key, packet);
		SendMonsterSetInfoPacket(key);
		break;
	case (BYTE)PacketType::PlayerMove:
		debugText = std::to_string(key) + " ProcessPacket::PlayerMove::Called!!";
		OnReceivePacket::PlayerMove(key, packet);
		break;
	case (BYTE)PacketType::PlayerLight:
		debugText = std::to_string(key) + " ProcessPacket::PlayerLight::Called!!";
		OnReceivePacket::PlayerLight(key, packet);
		break;
	case (BYTE)PacketType::PlayerShout:
		debugText = std::to_string(key) + " ProcessPacket::PlayerShout::Called!!";
		OnReceivePacket::PlayerShout(key, packet);
		break;
	case (BYTE)PacketType::PlayerGetItem:
		debugText = std::to_string(key) + " ProcessPacket::PlayerGetItem::Called!!";
		OnReceivePacket::PlayerGetItem(key, packet);
		break;
	case (BYTE)PacketType::MonsterMove:
		debugText = std::to_string(key) + " ProcessPacket::MonsterMove::Called!!";
		OnReceivePacket::MonsterMove(key, packet);
		break;
	default:
		debugText = std::to_string(key) + " ProcessPacket::Unknown Packet Type Detected";
		return;
	}
	DisplayDebugText(debugText);

	return;
}

void OnReceivePacket::Connect(int key, unsigned char* packet)
{
	std::string debugText = "";

	Packet::Connect *data = reinterpret_cast<Packet::Connect*>(packet);
	gClientsList[data->id].player.pos.x = data->posX;
	gClientsList[data->id].player.pos.y = data->posY;
	gClientsList[data->id].player.pos.z = data->posZ;

	// �ٸ� Ŭ���̾�Ʈ���� ���ο� Ŭ���̾�Ʈ �˸�
	for (int ci = 0; ci < MAX_USER; ++ci)
	{
		if (!gClientsList[ci].isConnect) continue;
		if (ci == key) continue;
		SendPacket(ci, packet);

		debugText = "ProcessPacket		:: " + std::to_string(ci) + " client <= " + std::to_string(key) + " client data";
		DisplayDebugText(debugText);
	}

	// ���� ���� Ŭ���̾�Ʈ���� �̹� �����ִ� �÷��̾� �������� �ѱ�
	Packet::Connect preOtherPlayerPacket;
	preOtherPlayerPacket.size = sizeof(Packet::Connect);
	preOtherPlayerPacket.type = (BYTE)PacketType::Connect;
	for (int ci = 0; ci < MAX_USER; ++ci)
	{
		if (!gClientsList[ci].isConnect) continue;
		if (ci == key) continue;
		preOtherPlayerPacket.id = ci;
		preOtherPlayerPacket.posX = gClientsList[ci].player.pos.x;
		preOtherPlayerPacket.posY = gClientsList[ci].player.pos.y;
		preOtherPlayerPacket.posZ = gClientsList[ci].player.pos.z;
		SendPacket(key, reinterpret_cast<unsigned char*>(&preOtherPlayerPacket));

		debugText = "ProcessPacket		:: "  + std::to_string(key) + " client <= " + std::to_string(ci) + " client data";
		DisplayDebugText(debugText);
	}

	return;
}

void OnReceivePacket::PlayerMove(int key, unsigned char* packet)
{
	int id = 0;
	Vector3 pos;

	Packet::Player::Move *data = reinterpret_cast<Packet::Player::Move*>(packet);

	id = data->id;
	pos.x = data->posX;
	pos.y = data->posY;
	pos.z = data->posZ;

	if (gClientsList[id].isConnect)
	{
		gClientsList[id].player.pos.x = pos.x;
		gClientsList[id].player.pos.y = pos.y;
		gClientsList[id].player.pos.z = pos.z;
	}
	
	std::string debugText = "OnReceivePlayerMovePacket::Pos("
		+ std::to_string(gClientsList[id].player.pos.x) + ", " + std::to_string(gClientsList[id].player.pos.y) + ", " + std::to_string(gClientsList[id].player.pos.z) + ")";
	DisplayDebugText(debugText);

	for (int ci = 0; ci < MAX_USER; ++ci)
	{
		if (!gClientsList[ci].isConnect) continue;
		if (gClientsList[ci].id == key) continue;
		SendPacket(ci, packet);
	}

	return;
}

void OnReceivePacket::PlayerLight(int key, unsigned char* packet)
{
	for (int ci = 0; ci < MAX_USER; ++ci)
	{
		if (!gClientsList[ci].isConnect) continue;
		if (gClientsList[ci].id == key) continue;
		SendPacket(ci, packet);
	}

	return;
}

void OnReceivePacket::PlayerShout(int key, unsigned char* packet)
{
	for (int ci = 0; ci < MAX_USER; ++ci)
	{
		if (!gClientsList[ci].isConnect) continue;
		if (gClientsList[ci].id == key) continue;
		SendPacket(ci, packet);
	}
	return;
}

void OnReceivePacket::PlayerGetItem(int key, unsigned char* packet)
{
	Packet::Player::GetItem *data = reinterpret_cast<Packet::Player::GetItem*>(packet);

	gItemArr[data->itemID] = data->id;

	std::string debugText = "OnReceivePlayer GetItem Packet:: " + std::to_string(data->id) + " get item " + std::to_string(data->itemID);
	DisplayDebugText(debugText);

	for (int ci = 0; ci < MAX_USER; ++ci)
	{
		if (!gClientsList[ci].isConnect) continue;
		if (gClientsList[ci].id == key) continue;
		SendPacket(ci, packet);
	}
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

	// ��ȭ�� �۴ٸ� �׳� �����մϴ�.
	if (retval) {
		gLock.unlock();
		return;
	}

	// ��ȭ�� ũ�ٸ� �� ������ �����մϴ�.
	gMonster.pos.x = data->posX;
	gMonster.pos.y = data->posY;
	gMonster.pos.z = data->posZ;

	std::string debugText = "OnReceiveMonsterMovePacket::gMonsterPos (" 
		+ std::to_string(gMonster.pos.x) + ", " + std::to_string(gMonster.pos.y) + ", " + std::to_string(gMonster.pos.z) + ")";
	gLock.unlock();
	DisplayDebugText(debugText);


	if (HasMonsterArrivedAtDestination())
	{
		SetMonsterNewPatrolPath();
		DisplayDebugText("True");
		gLock.lock();
		std::string debugText = "OnReceiveMonsterMovePacket::gMonsterPatrolPos ("
			+ std::to_string(gMonster.patrolPos.x) + ", " + std::to_string(gMonster.patrolPos.y) + ", " + std::to_string(gMonster.patrolPos.z) + ")";
		gLock.unlock();
		DisplayDebugText(debugText);
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

void InitializeItem(void)
{
	for (int count = 0; count < NUM_OF_ITEM; ++count)
		gItemArr[count] = NIL;
	return;
}

void SendMonsterSetInfoPacket(int key)
{
	// ���� ���� Ŭ���̾�Ʈ���� ���� ���� ����ȭ
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
	debugText = "ProcessPacket		:: " + std::to_string(key) + " client <= gMonster Pos(" 
		+ std::to_string((int)gMonster.pos.x) + ", " + std::to_string((int)gMonster.pos.y) + ", " + std::to_string((int)gMonster.pos.z) + ")";
	DisplayDebugText(debugText);
	
	debugText = "ProcessPacket		:: " + std::to_string(key) + " client <= gMonster Patrol Pos("
		+ std::to_string((int)gMonster.patrolPos.x) + ", " + std::to_string((int)gMonster.patrolPos.y) + ", " + std::to_string((int)gMonster.patrolPos.z) + ")";
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
	// rand �Լ� ���� �ʿ�
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

	for (int ci = 0; ci < MAX_USER; ++ci)
	{
		if (!gClientsList[ci].isConnect) continue;
		SendPacket(ci, reinterpret_cast<unsigned char*>(&packet));
	}
	return;
}