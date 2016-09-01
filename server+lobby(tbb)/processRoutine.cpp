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
		// ���ο� ���� ������ ����
		UpdateRoomInfo(id);
		break;
	case Notice::MAKE_ROOM:
		{
			bool makeRoom = false;
			int roomNo = NIL;

			while (!makeRoom)
			{
				roomNo = gRoomInfoMAP.Size() + 1;
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

					makeRoom = true;
				}
			}

			// ���� ���� client�� ���� �ִ� Room�� ��ȣ�� ���� ���� ��ȣ�� update�� �����Ѵ�.
			if (gClientInfoMAP.Contains(id)) {
				gClientInfoMAP.GetData(id)->player.roomNo = roomNo;

				std::cout << "Recv Notice Packet : MAKE_ROOM : Class No. of " << key << " is " << gClientInfoMAP.GetData(id)->player.roomNo << std::endl;
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
				// �������� �ϴ� client�� ������ ��
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
				// �������� �ϴ� client�� ������ �ƴ� ��
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

				// �������� ���ư� ���� �ڷᱸ�������� ����.
				gClientInfoMAP.GetData(id)->player.roomNo = 0;

				std::cout << "Recv Notice Packet : QUIT_ROOM : Class No. of " << key << " is " << gClientInfoMAP.GetData(id)->player.roomNo << std::endl;
			}

			// �������� ������ �Ǵ� ������ ���� ����� �ٽ� �޴´�.
			UpdateRoomInfo(key);
			// �ٸ� ���濡 �ִ� ���̵鿡�Ե� ����� ���ݾ� �ٲ���Ƿ�
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
				// ������ ��� ��� �÷��̾ �غ� ������ ���� GAME_READY�� ������.
				// ������ CANCEL_READY (x)
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