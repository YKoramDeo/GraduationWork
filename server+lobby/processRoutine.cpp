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
	case PacketType::JoinRoom:
		if (size != sizeof(Packet::JoinRoom))
			return false;
		break;
	/**********************변 경 사 항**********************/
	case PacketType::Connect:
		if (size != sizeof(Packet::Connect))
			return false;
		break;
	case PacketType::PlayerMove:
		if (size != sizeof(Packet::PlayerMove))
			return false;
		break;
	case PacketType::PlayerLight:
		if (size != sizeof(Packet::PlayerLight))
			return false;
		break;
	case PacketType::PlayerShout:
		if (size != sizeof(Packet::PlayerShout))
			return false;
		break;
	case PacketType::PlayerGetItem:
		if (size != sizeof(Packet::PlayerGetItem))
			return false;
		break;
	case PacketType::MonsterSetInfo:
		if (size != sizeof(Packet::MonsterSetInfo))
			return false;
		break;
	case PacketType::MonsterMove:
		if (size != sizeof(Packet::MonsterMove))
			return false;
		break;
	case PacketType::MonsterSetPatrolPos:
		if (size != sizeof(Packet::MonsterSetPatrolPos))
			return false;
		break;
	/**********************변 경 사 항**********************/
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
	case (BYTE)PacketType::JoinRoom:
		OnReceivePacket::JoinRoom(key, packet);
		break;
	/**********************변 경 사 항**********************/
	case (BYTE)PacketType::Connect:
		// Connect 동기화 하는 Packet
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
	/**********************변 경 사 항**********************/
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

	debugText = "ProcessPacket :: OnReceive Notify Packet :: Recv " + std::to_string(data->notice) + " from. " + std::to_string(data->id);
	//DisplayDebugText(debugText);

	switch (notice)
	{
	case Notice::RECV_SET_ID:
		// 새로운 방의 정보를 전달
		UpdateRoomInfo(key);
		break;
	case Notice::MAKE_ROOM:
		{
			RoomInfo roomInfo;
			roomInfo.no = gRoomInfoSet->GetIndex() + 1;
			roomInfo.chiefID = id;
			gRoomInfoSet->Add(roomInfo.no, roomInfo);

			// 새로 들어온 client의 현재 있는 Room의 번호를 현재 들어온 번호로 update를 시행한다.
			Client* client_ptr = nullptr;
			gClientInfoSet->Search(id, &client_ptr);
			Player playerData;
			playerData.roomNo = roomInfo.no;
			playerData.pos.x = DEFAULT_POS_X;
			playerData.pos.y = DEFAULT_POS_Y;
			playerData.pos.z = DEFAULT_POS_Z;
			gClientInfoSet->Update(id, playerData);

			std::cout << "Recv Notice Packet : MAKE_ROOM : Class No. of " << key << " is " << client_ptr->player.roomNo << std::endl;

			BroadcastingExceptIndex_With_UpdateRoomInfo(key);
		}
		break;
	case Notice::QUIT_ROOM:
		{
			Client* client_ptr = nullptr;
			gClientInfoSet->Search(id, &client_ptr);
			int roomNo = client_ptr->player.roomNo;

			RoomInfo *roomData_ptr = nullptr;
			if (gRoomInfoSet->Search(roomNo, &roomData_ptr))
			{
				// 나가고자 하는 client가 방장일 때
				if (id == roomData_ptr->chiefID)
				{
					if (roomData_ptr->partner_1_ID != NIL)
					{
						roomData_ptr->chiefID = roomData_ptr->partner_1_ID;
						roomData_ptr->partner_1_ID = NIL;
						roomData_ptr->partner_1_ready = false;

						Packet::RenewalRoomInfo renewalRoomInfoPacket;
						renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
						renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
						renewalRoomInfoPacket.roomNo = roomData_ptr->no;
						renewalRoomInfoPacket.chiefNo = roomData_ptr->chiefID;
						renewalRoomInfoPacket.partner_1_ID = roomData_ptr->partner_1_ID;
						renewalRoomInfoPacket.partner_2_ID = roomData_ptr->partner_2_ID;
						renewalRoomInfoPacket.partner_3_ID = roomData_ptr->partner_3_ID;
						renewalRoomInfoPacket.partner_1_ready = roomData_ptr->partner_1_ready;
						renewalRoomInfoPacket.partner_2_ready = roomData_ptr->partner_2_ready;
						renewalRoomInfoPacket.partner_3_ready = roomData_ptr->partner_3_ready;

						SendPacket(renewalRoomInfoPacket.chiefNo, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_1_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_2_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_3_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					}
					else if (roomData_ptr->partner_2_ID != NIL)
					{
						roomData_ptr->chiefID = roomData_ptr->partner_2_ID;
						roomData_ptr->partner_2_ID = NIL;
						roomData_ptr->partner_2_ready = false;

						Packet::RenewalRoomInfo renewalRoomInfoPacket;
						renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
						renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
						renewalRoomInfoPacket.roomNo = roomData_ptr->no;
						renewalRoomInfoPacket.chiefNo = roomData_ptr->chiefID;
						renewalRoomInfoPacket.partner_1_ID = roomData_ptr->partner_1_ID;
						renewalRoomInfoPacket.partner_2_ID = roomData_ptr->partner_2_ID;
						renewalRoomInfoPacket.partner_3_ID = roomData_ptr->partner_3_ID;
						renewalRoomInfoPacket.partner_1_ready = roomData_ptr->partner_1_ready;
						renewalRoomInfoPacket.partner_2_ready = roomData_ptr->partner_2_ready;
						renewalRoomInfoPacket.partner_3_ready = roomData_ptr->partner_3_ready;

						SendPacket(renewalRoomInfoPacket.chiefNo, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_1_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_2_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_3_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					}
					else if (roomData_ptr->partner_3_ID != NIL)
					{
						roomData_ptr->chiefID = roomData_ptr->partner_3_ID;
						roomData_ptr->partner_3_ID = NIL;
						roomData_ptr->partner_3_ready = false;

						Packet::RenewalRoomInfo renewalRoomInfoPacket;
						renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
						renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
						renewalRoomInfoPacket.roomNo = roomData_ptr->no;
						renewalRoomInfoPacket.chiefNo = roomData_ptr->chiefID;
						renewalRoomInfoPacket.partner_1_ID = roomData_ptr->partner_1_ID;
						renewalRoomInfoPacket.partner_2_ID = roomData_ptr->partner_2_ID;
						renewalRoomInfoPacket.partner_3_ID = roomData_ptr->partner_3_ID;
						renewalRoomInfoPacket.partner_1_ready = roomData_ptr->partner_1_ready;
						renewalRoomInfoPacket.partner_2_ready = roomData_ptr->partner_2_ready;
						renewalRoomInfoPacket.partner_3_ready = roomData_ptr->partner_3_ready;

						SendPacket(renewalRoomInfoPacket.chiefNo, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_1_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_2_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_3_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					}
					else {
						gRoomInfoSet->Remove(roomNo);
						BroadcastingExceptIndex_With_UpdateRoomInfo(key);
						gRoomInfoSet->CheckElement();
					}
				}
				// 나가고자 하는 client가 방장이 아닐 때
				else
				{
					if (id == roomData_ptr->partner_1_ID) {
						roomData_ptr->partner_1_ID = NIL;
						roomData_ptr->partner_1_ready = false;
					}
					else if (id == roomData_ptr->partner_2_ID) {
						roomData_ptr->partner_2_ID = NIL;
						roomData_ptr->partner_2_ready = false;
					}
					else {
						roomData_ptr->partner_3_ID = NIL;
						roomData_ptr->partner_3_ready = false;
					}

					Packet::RenewalRoomInfo renewalRoomInfoPacket;
					renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
					renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
					renewalRoomInfoPacket.roomNo = roomData_ptr->no;
					renewalRoomInfoPacket.chiefNo = roomData_ptr->chiefID;
					renewalRoomInfoPacket.partner_1_ID = roomData_ptr->partner_1_ID;
					renewalRoomInfoPacket.partner_2_ID = roomData_ptr->partner_2_ID;
					renewalRoomInfoPacket.partner_3_ID = roomData_ptr->partner_3_ID;
					renewalRoomInfoPacket.partner_1_ready = roomData_ptr->partner_1_ready;
					renewalRoomInfoPacket.partner_2_ready = roomData_ptr->partner_2_ready;
					renewalRoomInfoPacket.partner_3_ready = roomData_ptr->partner_3_ready;

					SendPacket(renewalRoomInfoPacket.chiefNo, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					if (renewalRoomInfoPacket.partner_1_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					if (renewalRoomInfoPacket.partner_2_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					if (renewalRoomInfoPacket.partner_3_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
				}

				// 새로 들어온 client의 현재 있는 Room의 번호를 현재 들어온 번호로 update를 시행한다.
				Player playerData;
				playerData.roomNo = 0;
				playerData.pos.x = DEFAULT_POS_X;
				playerData.pos.y = DEFAULT_POS_Y;
				playerData.pos.z = DEFAULT_POS_Z;
				gClientInfoSet->Update(id, playerData);

				std::cout << "Recv Notice Packet : QUIT_ROOM : Class No. of " << key << " is " << client_ptr->player.roomNo << std::endl;
			}

			// 대기방으로 나가게 되는 것으로 방의 목록을 다시 받는다.
			UpdateRoomInfo(key);
			// 다른 대기방에 있는 아이들에게도 목록이 조금씩 바뀌였으므로
			BroadcastingExceptIndex_With_UpdateRoomInfo(key);
		}
		break;
	case Notice::GAME_READY:
	case Notice::CANCEL_READY:
		{
			Client* client_ptr = nullptr;
			gClientInfoSet->Search(id, &client_ptr);
			int roomNo = client_ptr->player.roomNo;

			RoomInfo *roomData_ptr = nullptr;
			if (gRoomInfoSet->Search(roomNo, &roomData_ptr))
			{
				if (id != roomData_ptr->chiefID) {
					if (id == roomData_ptr->partner_1_ID)
						roomData_ptr->partner_1_ready = (notice == Notice::GAME_READY) ? true : false;
					else if (id == roomData_ptr->partner_2_ID)
						roomData_ptr->partner_2_ready = (notice == Notice::GAME_READY) ? true : false;
					else 
						roomData_ptr->partner_3_ready = (notice == Notice::GAME_READY) ? true : false;

					Packet::RenewalRoomInfo renewalRoomInfoPacket;
					renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
					renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
					renewalRoomInfoPacket.roomNo = roomData_ptr->no;
					renewalRoomInfoPacket.chiefNo = roomData_ptr->chiefID;
					renewalRoomInfoPacket.partner_1_ID = roomData_ptr->partner_1_ID;
					renewalRoomInfoPacket.partner_2_ID = roomData_ptr->partner_2_ID;
					renewalRoomInfoPacket.partner_3_ID = roomData_ptr->partner_3_ID;
					renewalRoomInfoPacket.partner_1_ready = roomData_ptr->partner_1_ready;
					renewalRoomInfoPacket.partner_2_ready = roomData_ptr->partner_2_ready;
					renewalRoomInfoPacket.partner_3_ready = roomData_ptr->partner_3_ready;

					SendPacket(renewalRoomInfoPacket.chiefNo, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					if (renewalRoomInfoPacket.partner_1_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					if (renewalRoomInfoPacket.partner_2_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					if (renewalRoomInfoPacket.partner_3_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
				}
				// 방장일 경우 모든 플레이어가 준비 상태일 때만 GAME_READY만 보낸다.
				// 방장은 CANCEL_READY (x)
				else
				{
					Packet::Notify gameStartPacket;
					gameStartPacket.size = sizeof(Packet::Notify);
					gameStartPacket.type = (BYTE)PacketType::Notify;
					gameStartPacket.id = NIL;
					gameStartPacket.notice = Notice::GAME_START;

					Client* client_ptr = nullptr;
					gClientInfoSet->Search(id, &client_ptr);
					int roomNo = client_ptr->player.roomNo;

					RoomInfo *roomData_ptr = nullptr;
					if (gRoomInfoSet->Search(roomNo, &roomData_ptr))
					{
						SendPacket(roomData_ptr->chiefID, reinterpret_cast<unsigned char*>(&gameStartPacket));
						if (roomData_ptr->partner_1_ID != NIL) SendPacket(roomData_ptr->partner_1_ID, reinterpret_cast<unsigned char*>(&gameStartPacket));
						if (roomData_ptr->partner_2_ID != NIL) SendPacket(roomData_ptr->partner_2_ID, reinterpret_cast<unsigned char*>(&gameStartPacket));
						if (roomData_ptr->partner_3_ID != NIL) SendPacket(roomData_ptr->partner_3_ID, reinterpret_cast<unsigned char*>(&gameStartPacket));
					}
				}
			}
			DisplayDebugText(debugText);
		}
		break;
	default:
		break;
	}
	return;
}

void OnReceivePacket::JoinRoom(int key, unsigned char* packet)
{
	std::string debugText = "";
	Packet::JoinRoom *data = reinterpret_cast<Packet::JoinRoom*>(packet);
	int id = key;
	int roomNo = data->roomNo;

	Client *client_ptr = nullptr;
	if (gClientInfoSet->Search(id, &client_ptr))
		client_ptr->player.roomNo = roomNo;

	bool ret = gRoomInfoSet->JoinClient(roomNo, id);

	if (ret) {
		RoomInfo *roomData = nullptr;
		gRoomInfoSet->Search(roomNo, &roomData);

		Packet::RenewalRoomInfo renewalRoomInfoPacket;
		renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
		renewalRoomInfoPacket.type = (BYTE)PacketType::RenewalRoomInfo;
		renewalRoomInfoPacket.roomNo = roomData->no;
		renewalRoomInfoPacket.chiefNo = roomData->chiefID;
		renewalRoomInfoPacket.partner_1_ID = roomData->partner_1_ID;
		renewalRoomInfoPacket.partner_2_ID = roomData->partner_2_ID;
		renewalRoomInfoPacket.partner_3_ID = roomData->partner_3_ID;
		renewalRoomInfoPacket.partner_1_ready = roomData->partner_1_ready;
		renewalRoomInfoPacket.partner_2_ready = roomData->partner_2_ready;
		renewalRoomInfoPacket.partner_3_ready = roomData->partner_3_ready;
		
		SendPacket(roomData->chiefID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
		if (roomData->partner_1_ID != NIL) SendPacket(roomData->partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
		if (roomData->partner_2_ID != NIL) SendPacket(roomData->partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
		if (roomData->partner_3_ID != NIL) SendPacket(roomData->partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));

		BroadcastingExceptIndex_With_UpdateRoomInfo(key);

		debugText = "ProcessPacket :: OnReceive JoinRoom Packet :: Join class #" + std::to_string(roomNo) + " from. " + std::to_string(key);
	}
	else {
		Packet::Notify joinFailPacket;
		joinFailPacket.size = sizeof(Packet::Notify);
		joinFailPacket.type = (BYTE)PacketType::Notify;
		joinFailPacket.notice = Notice::JOIN_FAIL;

		SendPacket(key, reinterpret_cast<unsigned char*>(&joinFailPacket));
		
		debugText = "ProcessPacket :: OnReceive JoinRoom Packet :: Fail!!";
	}
	DisplayDebugText(debugText);

	return;
}

/**********************변 경 사 항**********************/
void OnReceivePacket::Connect(int key, unsigned char* packet)
{
	std::string debugText = "";

	Packet::Connect *data = reinterpret_cast<Packet::Connect*>(packet);

	Client *clientData = nullptr;
	gClientInfoSet->Search(data->id, &clientData);
	
	clientData->player.pos.x = data->posX;
	clientData->player.pos.y = data->posY;
	clientData->player.pos.z = data->posZ;

	// 다른 클라이언트에게 새로운 클라이언트 알림
	BroadcastingExceptIndex(data->id, packet);

	// 새로 들어온 클라이언트에게 이미 들어와있던 플레이어 정보들을 넘김
	Packet::Connect preOtherPlayerPacket;
	preOtherPlayerPacket.size = sizeof(Packet::Connect);
	preOtherPlayerPacket.type = (BYTE)PacketType::Connect;

	bool result = false, marked = false;
	ClientNode *pred, *curr;
	ClientNode *tail = gClientInfoSet->GetTail();
	pred = gClientInfoSet->GetHead();
	curr = pred;
	while (curr->GetNext() != tail)
	{
		curr = pred->GetNextWithMark(&marked);
		if (marked) continue;

		result = gClientInfoSet->Contains(curr->id);
		if (result) {
			if (curr->data.isConnect && curr->id != key)
			{
				preOtherPlayerPacket.id = curr->id;
				preOtherPlayerPacket.posX = curr->data.player.pos.x;
				preOtherPlayerPacket.posY = curr->data.player.pos.y;
				preOtherPlayerPacket.posZ = curr->data.player.pos.z;
				SendPacket(key, reinterpret_cast<unsigned char*>(&preOtherPlayerPacket));
			}
			pred = curr;
		}
	}

	return;
}

void OnReceivePacket::PlayerMove(int key, unsigned char* packet)
{
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
	ClientNode *pred, *curr;
	ClientNode *tail = gClientInfoSet->GetTail();
	pred = gClientInfoSet->GetHead();
	curr = pred;
	while (curr->GetNext() != tail)
	{
		curr = pred->GetNextWithMark(&marked);
		if (marked) continue;

		result = gClientInfoSet->Contains(curr->id);
		if (result) {
			if (curr->data.isConnect && curr->id != key)
			{
				preOtherPlayerPacket.id = curr->id;
				preOtherPlayerPacket.posX = curr->data.player.pos.x;
				preOtherPlayerPacket.posY = curr->data.player.pos.y;
				preOtherPlayerPacket.posZ = curr->data.player.pos.z;
				SendPacket(key, reinterpret_cast<unsigned char*>(&preOtherPlayerPacket));
			}
			pred = curr;
		}
	}

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
	Packet::PlayerGetItem *data = reinterpret_cast<Packet::PlayerGetItem*>(packet);

	gItemArr[data->itemID] = data->id;

	std::string debugText = "OnReceivePlayer GetItem Packet:: " + std::to_string(data->id) + " get item " + std::to_string(data->itemID);
	DisplayDebugText(debugText);

	BroadcastingExceptIndex(key, packet);

	return;
}

void OnReceivePacket::MonsterMove(int key, unsigned char *packet)
{
	bool retval = true;

	Packet::MonsterMove *data = reinterpret_cast<Packet::MonsterMove*>(packet);

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

	std::string debugText = "OnReceiveMonsterMovePacket::gMonsterPos ("
		+ std::to_string(gMonster.pos.x) + ", " + std::to_string(gMonster.pos.y) + ", " + std::to_string(gMonster.pos.z) + ")";
	gLock.unlock();
	//DisplayDebugText(debugText);


	if (HasMonsterArrivedAtDestination())
	{
		SetMonsterNewPatrolPath();
		/*
		DisplayDebugText("True");
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
	monsterPath[0] = { 160.0f,	0.0f,	30.0f };
	monsterPath[1] = { 90.0f,	0.0f,	-60.0f };
	monsterPath[2] = { 160.0f,	0.0f,	-80.0f };
	monsterPath[3] = { 200.0f,	0.0f,	0.0f };

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
	// 새로 들어온 클라이언트에게 몬스터 정보 동기화
	Packet::MonsterSetInfo monsterSetInfoPacket;
	monsterSetInfoPacket.size = sizeof(Packet::MonsterSetInfo);
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
	//DisplayDebugText(debugText);

	debugText = "ProcessPacket		:: " + std::to_string(key) + " client <= gMonster Patrol Pos("
		+ std::to_string((int)gMonster.patrolPos.x) + ", " + std::to_string((int)gMonster.patrolPos.y) + ", " + std::to_string((int)gMonster.patrolPos.z) + ")";
	gLock.unlock();
	//DisplayDebugText(debugText);

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
	Packet::MonsterSetPatrolPos packet;
	packet.size = sizeof(Packet::MonsterSetPatrolPos);
	packet.type = (BYTE)PacketType::MonsterSetPatrolPos;
	gLock.lock();
	packet.posX = gMonster.patrolPos.x;
	packet.posY = gMonster.patrolPos.y;
	packet.posZ = gMonster.patrolPos.z;
	gLock.unlock();

	Broadcasting(reinterpret_cast<unsigned char*>(&packet));
	return;
}
/**********************변 경 사 항**********************/