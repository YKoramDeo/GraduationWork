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
			bool makeRoom = false;
			int roomNo = NIL;

			while (!makeRoom)
			{
				roomNo = (int)gRoomInfoMAP.Size() + 1;
				if (gRoomInfoMAP.Insert(roomNo))
				{
					gRoomInfoMAP.GetData(roomNo)->no = roomNo;
					gRoomInfoMAP.GetData(roomNo)->chiefID = id;
					gRoomInfoMAP.GetData(roomNo)->partner_1_ID = NIL;
					gRoomInfoMAP.GetData(roomNo)->partner_2_ID = NIL;
					gRoomInfoMAP.GetData(roomNo)->partner_3_ID = NIL;
					gRoomInfoMAP.GetData(roomNo)->partner_1_ready = false;
					gRoomInfoMAP.GetData(roomNo)->partner_2_ready = false;
					gRoomInfoMAP.GetData(roomNo)->partner_3_ready = false;
					for (int index = 0; index < NUM_OF_ITEM; ++index)
						gRoomInfoMAP.GetData(roomNo)->ItemList[index] = NIL;

					makeRoom = true;
				}
			}

			// 새로 들어온 client의 현재 있는 Room의 번호를 현재 들어온 번호로 update를 시행한다.
			if (gClientInfoMAP.Contains(id)) {
				gClientInfoMAP.GetData(id)->player.roomNo = roomNo;

				debugText = "Recv Notice Packet : MAKE_ROOM : Class No. of " + std::to_string(key) + " is " + std::to_string(gClientInfoMAP.GetData(id)->player.roomNo);
				DisplayDebugText(debugText);
			}

			BroadcastingExceptIndex_With_UpdateRoomInfo(key);
		}
		break;
	case Notice::QUIT_ROOM:
		{
			int roomNo = 0;
			if (gClientInfoMAP.Contains(id)) roomNo = gClientInfoMAP.GetData(id)->player.roomNo;
			
			if (gRoomInfoMAP.Contains(roomNo))
			{
				// 나가고자 하는 client가 방장일 때
				if (id == gRoomInfoMAP.GetData(roomNo)->chiefID)
				{
					if (gRoomInfoMAP.GetData(roomNo)->partner_1_ID != NIL)
					{
						gRoomInfoMAP.GetData(roomNo)->chiefID = gRoomInfoMAP.GetData(roomNo)->partner_1_ID;
						gRoomInfoMAP.GetData(roomNo)->partner_1_ID = NIL;
						gRoomInfoMAP.GetData(roomNo)->partner_1_ready = false;

						Packet::RenewalRoomInfo renewalRoomInfoPacket;
						renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
						renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
						renewalRoomInfoPacket.roomNo = gRoomInfoMAP.GetData(roomNo)->no;
						renewalRoomInfoPacket.chiefNo = gRoomInfoMAP.GetData(roomNo)->chiefID;
						renewalRoomInfoPacket.partner_1_ID = gRoomInfoMAP.GetData(roomNo)->partner_1_ID;
						renewalRoomInfoPacket.partner_2_ID = gRoomInfoMAP.GetData(roomNo)->partner_2_ID;
						renewalRoomInfoPacket.partner_3_ID = gRoomInfoMAP.GetData(roomNo)->partner_3_ID;
						renewalRoomInfoPacket.partner_1_ready = gRoomInfoMAP.GetData(roomNo)->partner_1_ready;
						renewalRoomInfoPacket.partner_2_ready = gRoomInfoMAP.GetData(roomNo)->partner_2_ready;
						renewalRoomInfoPacket.partner_3_ready = gRoomInfoMAP.GetData(roomNo)->partner_3_ready;

						SendPacket(renewalRoomInfoPacket.chiefNo, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_1_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_2_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_3_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					}
					else if (gRoomInfoMAP.GetData(roomNo)->partner_2_ID != NIL)
					{
						gRoomInfoMAP.GetData(roomNo)->chiefID = gRoomInfoMAP.GetData(roomNo)->partner_2_ID;
						gRoomInfoMAP.GetData(roomNo)->partner_2_ID = NIL;
						gRoomInfoMAP.GetData(roomNo)->partner_2_ready = false;

						Packet::RenewalRoomInfo renewalRoomInfoPacket;
						renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
						renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
						renewalRoomInfoPacket.roomNo = gRoomInfoMAP.GetData(roomNo)->no;
						renewalRoomInfoPacket.chiefNo = gRoomInfoMAP.GetData(roomNo)->chiefID;
						renewalRoomInfoPacket.partner_1_ID = gRoomInfoMAP.GetData(roomNo)->partner_1_ID;
						renewalRoomInfoPacket.partner_2_ID = gRoomInfoMAP.GetData(roomNo)->partner_2_ID;
						renewalRoomInfoPacket.partner_3_ID = gRoomInfoMAP.GetData(roomNo)->partner_3_ID;
						renewalRoomInfoPacket.partner_1_ready = gRoomInfoMAP.GetData(roomNo)->partner_1_ready;
						renewalRoomInfoPacket.partner_2_ready = gRoomInfoMAP.GetData(roomNo)->partner_2_ready;
						renewalRoomInfoPacket.partner_3_ready = gRoomInfoMAP.GetData(roomNo)->partner_3_ready;

						SendPacket(renewalRoomInfoPacket.chiefNo, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_1_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_2_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_3_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					}
					else if (gRoomInfoMAP.GetData(roomNo)->partner_3_ID != NIL)
					{
						gRoomInfoMAP.GetData(roomNo)->chiefID = gRoomInfoMAP.GetData(roomNo)->partner_3_ID;
						gRoomInfoMAP.GetData(roomNo)->partner_3_ID = NIL;
						gRoomInfoMAP.GetData(roomNo)->partner_3_ready = false;

						Packet::RenewalRoomInfo renewalRoomInfoPacket;
						renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
						renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
						renewalRoomInfoPacket.roomNo = gRoomInfoMAP.GetData(roomNo)->no;
						renewalRoomInfoPacket.chiefNo = gRoomInfoMAP.GetData(roomNo)->chiefID;
						renewalRoomInfoPacket.partner_1_ID = gRoomInfoMAP.GetData(roomNo)->partner_1_ID;
						renewalRoomInfoPacket.partner_2_ID = gRoomInfoMAP.GetData(roomNo)->partner_2_ID;
						renewalRoomInfoPacket.partner_3_ID = gRoomInfoMAP.GetData(roomNo)->partner_3_ID;
						renewalRoomInfoPacket.partner_1_ready = gRoomInfoMAP.GetData(roomNo)->partner_1_ready;
						renewalRoomInfoPacket.partner_2_ready = gRoomInfoMAP.GetData(roomNo)->partner_2_ready;
						renewalRoomInfoPacket.partner_3_ready = gRoomInfoMAP.GetData(roomNo)->partner_3_ready;

						SendPacket(renewalRoomInfoPacket.chiefNo, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_1_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_2_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
						if (renewalRoomInfoPacket.partner_3_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					}
					else {
						gRoomInfoMAP.Remove(roomNo);
						BroadcastingExceptIndex_With_UpdateRoomInfo(key);
					}
				}
				// 나가고자 하는 client가 방장이 아닐 때
				else
				{
					if (id == gRoomInfoMAP.GetData(roomNo)->partner_1_ID) {
						gRoomInfoMAP.GetData(roomNo)->partner_1_ID = NIL;
						gRoomInfoMAP.GetData(roomNo)->partner_1_ready = false;
					}
					else if (id == gRoomInfoMAP.GetData(roomNo)->partner_2_ID) {
						gRoomInfoMAP.GetData(roomNo)->partner_2_ID = NIL;
						gRoomInfoMAP.GetData(roomNo)->partner_2_ready = false;
					}
					else {
						gRoomInfoMAP.GetData(roomNo)->partner_3_ID = NIL;
						gRoomInfoMAP.GetData(roomNo)->partner_3_ready = false;
					}

					Packet::RenewalRoomInfo renewalRoomInfoPacket;
					renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
					renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
					renewalRoomInfoPacket.roomNo = gRoomInfoMAP.GetData(roomNo)->no;
					renewalRoomInfoPacket.chiefNo = gRoomInfoMAP.GetData(roomNo)->chiefID;
					renewalRoomInfoPacket.partner_1_ID = gRoomInfoMAP.GetData(roomNo)->partner_1_ID;
					renewalRoomInfoPacket.partner_2_ID = gRoomInfoMAP.GetData(roomNo)->partner_2_ID;
					renewalRoomInfoPacket.partner_3_ID = gRoomInfoMAP.GetData(roomNo)->partner_3_ID;
					renewalRoomInfoPacket.partner_1_ready = gRoomInfoMAP.GetData(roomNo)->partner_1_ready;
					renewalRoomInfoPacket.partner_2_ready = gRoomInfoMAP.GetData(roomNo)->partner_2_ready;
					renewalRoomInfoPacket.partner_3_ready = gRoomInfoMAP.GetData(roomNo)->partner_3_ready;

					SendPacket(renewalRoomInfoPacket.chiefNo, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					if (renewalRoomInfoPacket.partner_1_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					if (renewalRoomInfoPacket.partner_2_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
					if (renewalRoomInfoPacket.partner_3_ID != NIL) SendPacket(renewalRoomInfoPacket.partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
				}

				// 대기방으로 돌아간 것을 자료구조에서도 변경.
				gClientInfoMAP.GetData(id)->player.roomNo = 0;

				debugText = "Recv Notice Packet : QUIT_ROOM : Class No. of " + std::to_string(key) + " is " + std::to_string(gClientInfoMAP.GetData(id)->player.roomNo);
				DisplayDebugText(debugText);
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
			int roomNo = 0;
			if (gClientInfoMAP.Contains(id)) roomNo = gClientInfoMAP.GetData(id)->player.roomNo;

			if (gRoomInfoMAP.Contains(roomNo))
			{
				if (id != gRoomInfoMAP.GetData(roomNo)->chiefID) {
					if (id == gRoomInfoMAP.GetData(roomNo)->partner_1_ID)
						gRoomInfoMAP.GetData(roomNo)->partner_1_ready = (notice == Notice::GAME_READY) ? true : false;
					else if (id == gRoomInfoMAP.GetData(roomNo)->partner_2_ID)
						gRoomInfoMAP.GetData(roomNo)->partner_2_ready = (notice == Notice::GAME_READY) ? true : false;
					else 
						gRoomInfoMAP.GetData(roomNo)->partner_3_ready = (notice == Notice::GAME_READY) ? true : false;

					Packet::RenewalRoomInfo renewalRoomInfoPacket;
					renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
					renewalRoomInfoPacket.type = PacketType::RenewalRoomInfo;
					renewalRoomInfoPacket.roomNo = gRoomInfoMAP.GetData(roomNo)->no;
					renewalRoomInfoPacket.chiefNo = gRoomInfoMAP.GetData(roomNo)->chiefID;
					renewalRoomInfoPacket.partner_1_ID = gRoomInfoMAP.GetData(roomNo)->partner_1_ID;
					renewalRoomInfoPacket.partner_2_ID = gRoomInfoMAP.GetData(roomNo)->partner_2_ID;
					renewalRoomInfoPacket.partner_3_ID = gRoomInfoMAP.GetData(roomNo)->partner_3_ID;
					renewalRoomInfoPacket.partner_1_ready = gRoomInfoMAP.GetData(roomNo)->partner_1_ready;
					renewalRoomInfoPacket.partner_2_ready = gRoomInfoMAP.GetData(roomNo)->partner_2_ready;
					renewalRoomInfoPacket.partner_3_ready = gRoomInfoMAP.GetData(roomNo)->partner_3_ready;

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

					if (gRoomInfoMAP.Contains(roomNo))
					{
						SendPacket(gRoomInfoMAP.GetData(roomNo)->chiefID, reinterpret_cast<unsigned char*>(&gameStartPacket));
						if (gRoomInfoMAP.GetData(roomNo)->partner_1_ID != NIL) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_1_ID, reinterpret_cast<unsigned char*>(&gameStartPacket));
						if (gRoomInfoMAP.GetData(roomNo)->partner_2_ID != NIL) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_2_ID, reinterpret_cast<unsigned char*>(&gameStartPacket));
						if (gRoomInfoMAP.GetData(roomNo)->partner_3_ID != NIL) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_3_ID, reinterpret_cast<unsigned char*>(&gameStartPacket));
					
						debugText = "Send Notice Packet : Game Start Class." + std::to_string(gRoomInfoMAP.GetData(roomNo)->no);
						DisplayDebugText(debugText);
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

	gClientInfoMAP.GetData(id)->player.roomNo = roomNo;

	bool ret = false;

	if (gRoomInfoMAP.GetData(roomNo)->partner_1_ID == NIL) {
		gRoomInfoMAP.GetData(roomNo)->partner_1_ID = id;
		ret = true;
	}
	else if (gRoomInfoMAP.GetData(roomNo)->partner_2_ID == NIL) {
		gRoomInfoMAP.GetData(roomNo)->partner_2_ID = id;
		ret = true;
	}
	else if (gRoomInfoMAP.GetData(roomNo)->partner_3_ID == NIL) {
		gRoomInfoMAP.GetData(roomNo)->partner_3_ID = id;
		ret = true;
	}
	else
		ret = false;

	if (ret) {
		Packet::RenewalRoomInfo renewalRoomInfoPacket;
		renewalRoomInfoPacket.size = sizeof(Packet::RenewalRoomInfo);
		renewalRoomInfoPacket.type = (BYTE)PacketType::RenewalRoomInfo;
		renewalRoomInfoPacket.roomNo = gRoomInfoMAP.GetData(roomNo)->no;
		renewalRoomInfoPacket.chiefNo = gRoomInfoMAP.GetData(roomNo)->chiefID;
		renewalRoomInfoPacket.partner_1_ID = gRoomInfoMAP.GetData(roomNo)->partner_1_ID;
		renewalRoomInfoPacket.partner_2_ID = gRoomInfoMAP.GetData(roomNo)->partner_2_ID;
		renewalRoomInfoPacket.partner_3_ID = gRoomInfoMAP.GetData(roomNo)->partner_3_ID;
		renewalRoomInfoPacket.partner_1_ready = gRoomInfoMAP.GetData(roomNo)->partner_1_ready;
		renewalRoomInfoPacket.partner_2_ready = gRoomInfoMAP.GetData(roomNo)->partner_2_ready;
		renewalRoomInfoPacket.partner_3_ready = gRoomInfoMAP.GetData(roomNo)->partner_3_ready;
		
		SendPacket(gRoomInfoMAP.GetData(roomNo)->chiefID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
		if (gRoomInfoMAP.GetData(roomNo)->partner_1_ID != NIL) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_1_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
		if (gRoomInfoMAP.GetData(roomNo)->partner_2_ID != NIL) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_2_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));
		if (gRoomInfoMAP.GetData(roomNo)->partner_3_ID != NIL) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_3_ID, reinterpret_cast<unsigned char*>(&renewalRoomInfoPacket));

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

void OnReceivePacket::Connect(int key, unsigned char* packet)
{
	std::string debugText = "";

	Packet::Connect *data = reinterpret_cast<Packet::Connect*>(packet);

	debugText = "ProcessPacket :: OnReceive Connect Packet :: From." + std::to_string(key);
	DisplayDebugText(debugText);

	if (gClientInfoMAP.Contains(key))
	{
		gClientInfoMAP.GetData(key)->player.pos.x = data->posX;
		gClientInfoMAP.GetData(key)->player.pos.y = data->posY;
		gClientInfoMAP.GetData(key)->player.pos.z = data->posZ;
	}

	// 방장일 경우 방장이라는 정보를 보내준다.
	int roomNo = gClientInfoMAP.GetData(key)->player.roomNo;
	if (gRoomInfoMAP.GetData(roomNo)->chiefID == data->id)
	{
		Packet::Notify youAreChiefPacket;
		youAreChiefPacket.size = sizeof(Packet::Notify);
		youAreChiefPacket.type = (BYTE)PacketType::Notify;
		youAreChiefPacket.id = NIL;
		youAreChiefPacket.notice = Notice::YOU_ARE_CHIEF;
		SendPacket(key, reinterpret_cast<unsigned char*>(&youAreChiefPacket));
		Sleep(10);
	}
	// 방에 진입한 클라이언트들에게 방에 참여한 인원들에 대한 정보를 넘김
	Packet::Connect otherPlayerPacket;
	otherPlayerPacket.size = sizeof(Packet::Connect);
	otherPlayerPacket.type = (BYTE)PacketType::Connect;

	if (key != gRoomInfoMAP.GetData(roomNo)->chiefID)
	{
		otherPlayerPacket.id = gRoomInfoMAP.GetData(roomNo)->chiefID;
		otherPlayerPacket.posX = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.x;
		otherPlayerPacket.posY = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.y;
		otherPlayerPacket.posZ = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.z;
		SendPacket(key, reinterpret_cast<unsigned char*>(&otherPlayerPacket));
		debugText = "Send Previous " + std::to_string(otherPlayerPacket.id) + " OtherPlayer Data To." + std::to_string(key);
		DisplayDebugText(debugText);
		Sleep(10);
	}
	if (key != gRoomInfoMAP.GetData(roomNo)->partner_1_ID && gRoomInfoMAP.GetData(roomNo)->partner_1_ID != NIL)
	{
		otherPlayerPacket.id = gRoomInfoMAP.GetData(roomNo)->partner_1_ID;
		otherPlayerPacket.posX = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.x;
		otherPlayerPacket.posY = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.y;
		otherPlayerPacket.posZ = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.z;
		SendPacket(key, reinterpret_cast<unsigned char*>(&otherPlayerPacket));
		debugText = "Send Previous " + std::to_string(otherPlayerPacket.id) + " OtherPlayer Data To." + std::to_string(key);
		DisplayDebugText(debugText);
		Sleep(10);
	}
	if (key != gRoomInfoMAP.GetData(roomNo)->partner_2_ID && gRoomInfoMAP.GetData(roomNo)->partner_2_ID != NIL)
	{
		otherPlayerPacket.id = gRoomInfoMAP.GetData(roomNo)->partner_2_ID;
		otherPlayerPacket.posX = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.x;
		otherPlayerPacket.posY = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.y;
		otherPlayerPacket.posZ = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.z;
		SendPacket(key, reinterpret_cast<unsigned char*>(&otherPlayerPacket));
		debugText = "Send Previous " + std::to_string(otherPlayerPacket.id) + " OtherPlayer Data To." + std::to_string(key);
		DisplayDebugText(debugText);
		Sleep(10);
	}
	if (key != gRoomInfoMAP.GetData(roomNo)->partner_3_ID && gRoomInfoMAP.GetData(roomNo)->partner_3_ID != NIL)
	{
		otherPlayerPacket.id = gRoomInfoMAP.GetData(roomNo)->partner_3_ID;
		otherPlayerPacket.posX = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.x;
		otherPlayerPacket.posY = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.y;
		otherPlayerPacket.posZ = gClientInfoMAP.GetData(otherPlayerPacket.id)->player.pos.z;
		SendPacket(key, reinterpret_cast<unsigned char*>(&otherPlayerPacket));
		debugText = "Send Previous " + std::to_string(otherPlayerPacket.id) + " OtherPlayer Data To." + std::to_string(key);
		DisplayDebugText(debugText);
		Sleep(10);
	}

	return;
}

void OnReceivePacket::PlayerMove(int key, unsigned char* packet)
{
	Packet::Connect *data = reinterpret_cast<Packet::Connect*>(packet);
	
	if (gClientInfoMAP.Contains(key))
	{
		gClientInfoMAP.GetData(key)->player.pos.x = data->posX;
		gClientInfoMAP.GetData(key)->player.pos.y = data->posY;
		gClientInfoMAP.GetData(key)->player.pos.z = data->posZ;
	}

	// 다른 클라이언트에게 새로운 클라이언트 알림

	BroadcastingExceptIndex(key, packet);

	std::string debugText = "OnReceivePlayer PlayerMove Packet:: " + std::to_string(data->posX) + ", " + std::to_string(data->posY) + ", " + std::to_string(data->posZ) + " from." + std::to_string(data->id);
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

	int roomNo = gClientInfoMAP.GetData(data->id)->player.roomNo;
	gRoomInfoMAP.GetData(roomNo)->ItemList[data->itemID] = data->id;

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

	int roomNo = gClientInfoMAP.GetData(key)->player.roomNo;

	if (gRoomInfoMAP.GetData(roomNo)->partner_1_ID != NIL) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_1_ID, packet);
	if (gRoomInfoMAP.GetData(roomNo)->partner_2_ID != NIL) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_2_ID, packet);
	if (gRoomInfoMAP.GetData(roomNo)->partner_3_ID != NIL) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_3_ID, packet);

	std::string debugText = "OnReceiveMonsterMovePacket:: Send Monster Status : "
		+ std::to_string(data->status) + ", Pos (" + std::to_string(data->posX) + ", " + std::to_string(data->posY) + ", " + std::to_string(data->posZ) + ")";

	//DisplayDebugText(debugText);

	return;
}

void OnReceivePacket::MonsterAI(int key, unsigned char *packet)
{
	bool retval = true;

	Packet::MonsterAI *data = reinterpret_cast<Packet::MonsterAI*>(packet);

	int roomNo = gClientInfoMAP.GetData(key)->player.roomNo;

	if (gRoomInfoMAP.GetData(roomNo)->chiefID != key) SendPacket(gRoomInfoMAP.GetData(roomNo)->chiefID, packet);
	if (gRoomInfoMAP.GetData(roomNo)->partner_1_ID != NIL && gRoomInfoMAP.GetData(roomNo)->partner_1_ID != key) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_1_ID, packet);
	if (gRoomInfoMAP.GetData(roomNo)->partner_2_ID != NIL && gRoomInfoMAP.GetData(roomNo)->partner_2_ID != key) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_2_ID, packet);
	if (gRoomInfoMAP.GetData(roomNo)->partner_3_ID != NIL && gRoomInfoMAP.GetData(roomNo)->partner_3_ID != key) SendPacket(gRoomInfoMAP.GetData(roomNo)->partner_3_ID, packet);

	std::string debugText = "OnReceiveMonsterAIPacket:: Send Monster Status : "
		+ std::to_string(data->status) + ", Last Pos (" + std::to_string(data->lastPosX) + ", " + std::to_string(data->lastPosY) + ", " + std::to_string(data->lastPosZ) + ")";

	DisplayDebugText(debugText);

	return;
}