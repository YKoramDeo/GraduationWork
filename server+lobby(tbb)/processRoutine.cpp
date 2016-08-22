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
	DisplayDebugText(debugText);

	switch (notice)
	{
	case Notice::RECV_SET_ID:
		// 새로운 방의 정보를 전달
		UpdateRoomInfo(id);
		break;
	case Notice::MAKE_ROOM:
		{
			RoomInfo roomInfo;
			roomInfo.no = gRoomInfoSet->GetIndex() + 1;
			roomInfo.chiefID = id;
			gRoomInfoSet->Add(roomInfo.no, roomInfo);

			// 새로 들어온 client의 현재 있는 Room의 번호를 현재 들어온 번호로 update를 시행한다.
			if (gClientInfoMAP.Contains(id)) {
				gClientInfoMAP.GetData(id)->player.roomNo = roomInfo.no;

				std::cout << "Recv Notice Packet : MAKE_ROOM : Class No. of " << key << " is " << gClientInfoMAP.GetData(id)->player.roomNo << std::endl;
			}

			BroadcastingExceptIndex_With_UpdateRoomInfo(key);
		}
		break;
	case Notice::QUIT_ROOM:
		{
			Client* client_ptr = nullptr;
			//gClientInfoSet->Search(id, &client_ptr);
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
				//gClientInfoSet->Update(id, playerData);

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
			//gClientInfoSet->Search(id, &client_ptr);
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
					//gClientInfoSet->Search(id, &client_ptr);
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
	//if (gClientInfoSet->Search(id, &client_ptr))
	//	client_ptr->player.roomNo = roomNo;

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