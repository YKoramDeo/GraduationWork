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
	case PacketType::MonsterMove:
		if (size != sizeof(Packet::MonsterMove))
			return false;
		break;
	case PacketType::MonsterAI:
		if (size != sizeof(Packet::MonsterAI))
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
	case (BYTE)PacketType::MonsterAI:
		debugText = std::to_string(key) + " ProcessPacket::MonsterAI::Called!!";
		OnReceivePacket::MonsterAI(key, packet);
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
			//gRoomInfoSet->CheckElement();
			//std::cout << "room No." << roomInfo.no << ", cheif ID." << roomInfo.chiefID << std::endl;

			// 새로 들어온 client의 현재 있는 Room의 번호를 현재 들어온 번호로 update를 시행한다.
			Client* client_ptr = nullptr;
			gClientInfoSet->Search(id, &client_ptr);
			Player playerData;
			playerData.roomNo = roomInfo.no;
			playerData.pos.x = DEFAULT_POS_X;
			playerData.pos.y = DEFAULT_POS_Y;
			playerData.pos.z = DEFAULT_POS_Z;
			gClientInfoSet->Update(id, playerData);

			std::cout << "Recv Notice Packet : MAKE_ROOM : Class No." << client_ptr->player.roomNo << " by " << key << std::endl;

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

						std::cout << "Send Notice Packet : Game Start Class."  << roomData_ptr->no << std::endl;
					}
				}
			}
			//DisplayDebugText(debugText);
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

	debugText = "ProcessPacket :: OnReceive Connect Packet :: From." + std::to_string(key);
	DisplayDebugText(debugText);

	Client *clientData = nullptr;
	gClientInfoSet->Search(data->id, &clientData);
	
	clientData->player.pos.x = data->posX;
	clientData->player.pos.y = data->posY;
	clientData->player.pos.z = data->posZ;

	// 다른 클라이언트에게 새로운 클라이언트 알림
	//BroadcastingExceptIndex(data->id, packet);

	// 방장일 경우 방장이라는 정보를 보내준다.
	RoomInfo *roomData = nullptr;
	gRoomInfoSet->Search(clientData->player.roomNo, &roomData);
	if (roomData->chiefID == data->id)
	{
		Packet::Notify youAreChiefPacket;
		youAreChiefPacket.size = sizeof(Packet::Notify);
		youAreChiefPacket.type = (BYTE)PacketType::Notify;
		youAreChiefPacket.id = NIL;
		youAreChiefPacket.notice = Notice::YOU_ARE_CHIEF;
		SendPacket(key, reinterpret_cast<unsigned char*>(&youAreChiefPacket));
		Sleep(100);
	}
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
				std::cout << "Send Previous "<< curr->id << " OtherPlayer Data To." << key << std::endl;
				Sleep(10);
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
	Client *client_ptr = nullptr;
	gClientInfoSet->Search(key, &client_ptr);

	transmittedPlayerData.roomNo = client_ptr->player.roomNo;
	transmittedPlayerData.pos.x = data->posX;
	transmittedPlayerData.pos.y = data->posY;
	transmittedPlayerData.pos.z = data->posZ;

	gClientInfoSet->Update(key, transmittedPlayerData);

	// 다른 클라이언트에게 새로운 클라이언트 알림

	BroadcastingExceptIndex(key, packet);
	
	//std::string debugText = "OnReceivePlayer PlayerMove Packet:: " + std::to_string(data->posX) + ", " + std::to_string(data->posY) + ", " + std::to_string(data->posZ) + " from." + std::to_string(data->id);
	//DisplayDebugText(debugText);

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
	// 이 함수는 방장인 client에게만 받아서 처리하는 함수
	bool retval = true;

	Packet::MonsterMove *data = reinterpret_cast<Packet::MonsterMove*>(packet);

	Client *clientData = nullptr;
	gClientInfoSet->Search(key, &clientData);

	RoomInfo *roomData = nullptr;
	gRoomInfoSet->Search(clientData->player.roomNo, &roomData);
	
	if (roomData->partner_1_ID != NIL) SendPacket(roomData->partner_1_ID, packet);
	if (roomData->partner_2_ID != NIL) SendPacket(roomData->partner_2_ID, packet);
	if (roomData->partner_3_ID != NIL) SendPacket(roomData->partner_3_ID, packet);

	std::string debugText = "OnReceiveMonsterMovePacket:: Send Monster Status : "
		+ std::to_string(data->status)  + ", Pos (" + std::to_string(data->posX) + ", " + std::to_string(data->posY) +  ", " + std::to_string(data->posZ) + ")";

	//DisplayDebugText(debugText);

	return;
}

void OnReceivePacket::MonsterAI(int key, unsigned char *packet)
{
	bool retval = true;

	Packet::MonsterAI *data = reinterpret_cast<Packet::MonsterAI*>(packet);

	Client *clientData = nullptr;
	gClientInfoSet->Search(key, &clientData);

	RoomInfo *roomData = nullptr;
	gRoomInfoSet->Search(clientData->player.roomNo, &roomData);

	if (roomData->chiefID != key) SendPacket(roomData->chiefID, packet);
	if (roomData->partner_1_ID != NIL && roomData->partner_1_ID != key) SendPacket(roomData->partner_1_ID, packet);
	if (roomData->partner_2_ID != NIL && roomData->partner_2_ID != key) SendPacket(roomData->partner_2_ID, packet);
	if (roomData->partner_3_ID != NIL && roomData->partner_3_ID != key) SendPacket(roomData->partner_3_ID, packet);

	std::string debugText = "OnReceiveMonsterAIPacket:: Send Monster Status : "
		+ std::to_string(data->status) + ", Last Pos (" + std::to_string(data->lastPosX) + ", " + std::to_string(data->lastPosY) + ", " + std::to_string(data->lastPosZ) + ")";

	DisplayDebugText(debugText);

	return;
}

void InitializeItem(void)
{
	for (int count = 0; count < NUM_OF_ITEM; ++count)
		gItemArr[count] = NIL;
	return;
}
/**********************변 경 사 항**********************/