#include "iocpNetwork.h"

void DisplayErrMsg(char* msg, int errNo)
{
	WCHAR *lpMsgBuf;
	// FormatMessage() : �����ڵ忡 �����ϴ� ���� �޽����� ���� �� �ִ�.
	// msdn - The function finds the message definition in a message table resource vased on a message identifier and a language identifier.
	//		The funtion copies the formatted message text to an output buffer, processing ant embadded insert sequences if requested.

	// FORMAT_MESSAGE_ALLOCATE_BUFFER			: ���� �޽����� ������ ������ �Լ��� �˾Ƽ� �Ҵ��Ѵٴ� �ǹ�
	// FORMAT_MESSAGE_FROM_SYSTEN				: OS�κ��� ���� �޽����� �����´ٴ� �ǹ�

	// MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT)	: ����ڰ� �����ǿ��� ������ �⺻���� �����޽��� ���� �� ����
	// lpMsgBuf �� ���� �޽����� ������ �����ε� �Լ��� �˾Ƽ� �Ҵ�, 
	// ���� �޽��� ����� ��ġ�� LocalFree() �Լ��� �̿��� �Ҵ��� �޸� ��ȯ �ʼ�
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errNo, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg); wprintf(L" %d Error::%s\n", errNo, lpMsgBuf);
	LocalFree(lpMsgBuf);
	return;
}

void DisplayDebugText(std::string str)
{
	std::cout << str << std::endl;
	return;
}

void InitializeServer(void)
{
	WSADATA wsadata;
	_wsetlocale(LC_ALL, L"korean");

	// Initialize Winsock
	// WSAStartup() :  ���α׷����� ����� ��� ������ ��û�����ν� ���� ���̺귯��(WS2_32.DLL)�� �ʱ�ȭ �ϴ� ����
	// msdn - The WSAStartup function initiates use of Winsock DLL bt a process

	// MAKEWORD(2,2)	: ���α׷��� �䱸�ϴ� �ֻ��� ���� ����, ���� 8��Ʈ�� �� ����, ���� 8��Ʈ�� �� ������ �־ ����
	//					: ���� ���� 3.2 ������ ����� ��û�Ѵٸ� 0x0203 �Ǵ� MAKEWORD(3,2)�� ���
	// wsadata			: ������ �ü���� �����ϴ� ���� ������ ���� ����(���α׷��� ������ ����ϰ� �� ���� ����,
	//					: �ý����� �����ϴ� ���� �ֻ��� ���� ��)�� ���� �� ����. ������ ������ �̷� ���� ��� X
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		DisplayDebugText("InitializeServer :: WSAStartup Fail !!");
		exit(1);
	}

	// Create IOCP kernel object
	// if last argument is 0, Use kernel object by core count
	// CreateIoCompletionPort() : ����� �Ϸ� ��Ʈ�� ���� ����.
	//							: ����� �׷� ��Ʈ�� �񵿱� ����� ����� �� ����� ó���� �����忡 ���� ������ ��� �ִ� ����
	// msdn - Creates an input/output (I/O) completion port and associates it with a specified file handle, 
	//		or reates an I/O completion port that is not yet associated with a file handle, allowing association at a later time.
	ghIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == ghIOCP)
	{
		DisplayDebugText("InitializeServer :: CreateIoCompletionPort Fail !!");
		exit(1);
	}

	return;
}

void StopServer(void)
{
	gClientInfoSet->Initialize();

	WSACleanup();
	DisplayDebugText("Stop Server :: Called!!");
	return;
}

void WorkerThreadFunc(void)
{
	DWORD ioSize, key;
	OverlapEx *overlap;
	BOOL result;
	Client *clientData = nullptr;

	while (true)
	{
		result = GetQueuedCompletionStatus(ghIOCP, &ioSize, reinterpret_cast<PULONG_PTR>(&key), reinterpret_cast<LPOVERLAPPED*>(&overlap), INFINITE);

		if (false == result || 0 == ioSize)
		{
			if (false == result)
				DisplayErrMsg("WorkerThread :: GetQueuedCompletionStatus Fail !!", GetLastError());

			if (WSAENOTSOCK == GetLastError() || ERROR_NETNAME_DELETED == GetLastError() || 0 == ioSize)
			{
				gClientInfoSet->Search(key, &clientData);
				clientData->isConnect = false;
				closesocket(clientData->socket);

				// ToDo : 10054 error ó��
				// �� �÷��̾�� �÷��̾� ���� ���� �˸�
				Packet::Disconnect packet;
				packet.size = sizeof(Packet::Disconnect);
				packet.type = PacketType::Disconnect;
				packet.id = key;

				BroadcastingExceptIndex(key, reinterpret_cast<unsigned char*>(&packet));
				gClientInfoSet->Remove(key);

				std::string debugText = "WorkerThread :: Disconnect " + std::to_string(key) + " client. :(";
				DisplayDebugText(debugText);
			}
			continue;
		}
		if (OP_RECV == overlap->operation)
		{
			gClientInfoSet->Search(key, &clientData);
			
			unsigned char *buf_ptr = overlap->buffer;
			int remained = ioSize;
			while (0 < remained)
			{
				if (0 == clientData->recvOverlap.packetSize)
					clientData->recvOverlap.packetSize = buf_ptr[0];
				int required = clientData->recvOverlap.packetSize - clientData->previousDataSize;
				// ��Ŷ�� �ϼ� ��ų �� �ִ°�? ���°�?
				if (remained >= required)
				{
					// �ϼ��� ��ų �� �ִ� ��Ȳ�̸�
					// ��Ŷ�� ��� �������ٰ� ���� ��ſ;� �Ѵ�.
					// �׷��� ��Ŷ�� �ϼ���Ű�� ��������� ������ �־���Ѵ�. 
					// �����Ͱ� ��Ŷ������ ���� ���� �ƴϱ� ������ ��Ŷ������ ó���ϰ� ���������ʹ� �� ������ ������ �����ؾ�
					// ������ �� �����Ͱ� �������� ���� ä�� ���ԵǸ� ������ ������ ����ְ� �ϳ��� ��Ŷ���� ���� ����� �־�� �Ѵ�.
					memcpy(clientData->packetBuf + clientData->previousDataSize, buf_ptr, required);
					// +�ϴ� ������ �������� ���� ������ ���Ŀ� ������ �ؾ��ϱ� ������ �� ������ġ�� �Ű���.
					ProcessPacket(key, clientData->packetBuf);
					// Packet ó��
					remained -= required;
					// ���� �ִ� ���� �ʿ��� ���� �����ϰ�
					buf_ptr += required;
					// ???
					clientData->recvOverlap.packetSize = 0;
				}
				else
				{
					// ��Ŷ�� �ϼ� ��ų �� ���� ũ���̴�.
					memcpy(clientData->packetBuf + clientData->previousDataSize, buf_ptr, remained);
					// buf_ptr�� ��� ���� packet�� �����Ѵ�.
					clientData->previousDataSize += remained;
					// ������ �����Ͱ� �����ִ� �����ŭ �þ��.
					remained = 0;
					// ���� recv�� �����ִ� size�� �̹� ������ �صξ����Ƿ� �ʱ�ȭ
				}
			}
			DWORD flags = 0;
			WSARecv(clientData->socket,
				&clientData->recvOverlap.wsaBuf,
				1, NULL, &flags,
				reinterpret_cast<LPWSAOVERLAPPED>(&clientData->recvOverlap),
				NULL);
		}
		else if (OP_SEND == overlap->operation)
		{
			// ioSize�ϰ� ���� ���� ũ�� �� �� ���� ���� ����
			BYTE packetType = overlap->buffer[1];
			BYTE packetSize = overlap->buffer[0];

			if (BeCompeletedSendPacket(packetType, packetSize))
				delete overlap;
			else
			{
				std::string debugText = "WorkerThread :: " + std::to_string(key) + " client don't send " + std::to_string(packetType) + "No. packet";
				DisplayDebugText(debugText);
			}
		}
		else
		{
			DisplayDebugText("WorkerThread :: Unknown Event on worker_thread");
			continue;
		}
	}
	return;
}

void AcceptThreadFunc(void)
{
	int retval = 0;
	struct sockaddr_in listenAddr;
	int newID = -1;

	SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == acceptSocket)
	{
		DisplayErrMsg("AcceptThread :: acceptSocket Fail !!", WSAGetLastError());
		return;
	}

	// bind() : ������ ���� IP�ּҿ� ���� ��Ʈ ��ȣ�� ����, C++11�� bind��� �Լ��� ȥ�� ������::����Ȯ�� �����ڿ� ���� ���.
	// msdn - The bind functio associates a local address with a socket

	// listenAddr	: ���� �ּ� ����ü�� ���� IP�ּҿ� ���� ��Ʈ ��ȣ�� �ʱ�ȭ�Ͽ� �����Ѵ�.
	// AF_INET		: ���ͳ� ���� ü��(IPv4)�� ����Ѵٴ� �ǹ�
	// INADDR_ANY	: ������ ���� IP�ּҸ� �����ϴµ� �־� ������ ���� ������ �ϸ� ������ IP�ּҸ� 2 �� �̻� ������ ��� 
	//				(multihomed host��� �θ�), Ŭ���̾�Ʈ�� ��� IP �ּҷ� �����ϵ� �޾Ƶ��� �� ����
	// bind �Լ��� 2��° ���ڴ� �׻� (SOCKADDR*)������ ��ȯ�ؾ� ��
	ZeroMemory(&listenAddr, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenAddr.sin_port = htons(MY_SERVER_PORT);
	ZeroMemory(&listenAddr.sin_zero, 8);
	retval = ::bind(acceptSocket, reinterpret_cast<sockaddr*>(&listenAddr), sizeof(listenAddr));
	if (SOCKET_ERROR == retval)
	{
		DisplayErrMsg("AcceptThread :: bind Fail !!", WSAGetLastError());
		exit(1);
	}
	// listen() : ������ TCP ��Ʈ ���¸� LISTENING���� �ٲ۴�. �̴� Ŭ���̾�Ʈ ������ �޾Ƶ��� �� �ִ� ���°� ���� �ǹ�
	// msdn - The listen function places a socket in a state in which it is listening for an incomming connection.

	// SOMAXCONN : Ŭ���̾�Ʈ�� ���� ������ ���� ť�� ����Ǵµ�, backlog�� �� ���� ť�� ���̸� ��Ÿ��.
	//			SOMAXCONN�� �ǹ̴� �Ϻ� �������ݿ��� ���� ������ �ִ��� ���
	listen(acceptSocket, SOMAXCONN);
	if (SOCKET_ERROR == retval)
	{
		DisplayErrMsg("AcceptThread :: listen Fail !!", WSAGetLastError());
		exit(1);
	}

	while (true)
	{
		struct sockaddr_in clientAddr;
		int addrSize = sizeof(clientAddr);

		// ������ Ŭ���̾�Ʈ�� �� ID

		// accept() : ������ Ŭ���̾�Ʈ�� ����� �� �ֵ��� ���ο� ������ �����ؼ� ���� ���� ������ Ŭ���̾�Ʈ�� �ּ������� �˷���
		//			: �������忡���� ���� IP�ּҿ� ���� ��Ʈ��ȣ, Ŭ���̾�Ʈ ���忡���� ���� IP �ּҿ� ���� ��Ʈ ��ȣ
		// WSAAccept() : ?
		// msdn - The WSAAccept function conditionally accepts a connection based on the return value of a condition,
		//		provides quality of service flow specifications, and allows the transfer of connection data.
		SOCKET newClientSocket = WSAAccept(acceptSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrSize, NULL, NULL);
		if (INVALID_SOCKET == newClientSocket)
		{
			DisplayErrMsg("AcceptThread :: WSAAccept Fail !!", WSAGetLastError());
			break;
		}

		// ������ Ŭ���̾�Ʈ�� ���ο� ID �ο��ϴ� ���� 
		newID++;

		// Accept�� ���� ���Ŀ� ����� ���־�� ��
		HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(newClientSocket), ghIOCP, newID, 0);
		if (NULL == result)
		{
			DisplayErrMsg("AcceptThread :: CreateIoCompletionPort Fail !!", WSAGetLastError());
			closesocket(newClientSocket);
			continue;
		}

		gClientInfoSet->Add(newID);
		Player playerData;
		playerData.roomNo = 0;
		playerData.pos.x = DEFAULT_POS_X;
		playerData.pos.y = DEFAULT_POS_Y;
		playerData.pos.z = DEFAULT_POS_Z;
		gClientInfoSet->Update(newID, playerData);
		
		Client *clientData = nullptr;
		gClientInfoSet->Search(newID, &clientData);
		clientData->socket = newClientSocket;
		
		// error code 64�� �ذ��ϱ� ���� delay�� ��.
		Sleep(10);

		// ���ο� Ŭ���̾�Ʈ ���� �˸�
		Packet::SetID clientSetIDPacket;
		clientSetIDPacket.size = sizeof(Packet::SetID);
		clientSetIDPacket.type = (BYTE)PacketType::SetID;
		clientSetIDPacket.id = newID;
		SendPacket(newID, reinterpret_cast<unsigned char*>(&clientSetIDPacket));

		// ���ο� ���� Recv ����
		DWORD flags = 0;
		// WSARecv() : 5��°, 6��° �� ���ڸ� NULL ������ ����ϸ� recv() �Լ�ó�� ���� �Լ��� ����
		retval = WSARecv(newClientSocket, &clientData->recvOverlap.wsaBuf, 1, NULL, &flags, &clientData->recvOverlap.originalOverlap, NULL);
		if (0 != retval)
		{
			int errNo = WSAGetLastError();
			if (WSA_IO_PENDING != errNo)
			{
				DisplayErrMsg("AcceptThread :: WSARecv", errNo);
			}
		}

		// Output
		std::string debugText = "AcceptThread :: ID " + std::to_string(newID) + " client Accept Success !!";
		DisplayDebugText(debugText);
	}

	return;
}

void SendPacket(const int index, const unsigned char* packet)
{
	int retval = 0;
	OverlapEx *sendOverlap = new OverlapEx;
	memset(sendOverlap, 0, sizeof(OverlapEx));
	sendOverlap->operation = OP_SEND;
	memset(&sendOverlap->originalOverlap, 0, sizeof(WSAOVERLAPPED));
	ZeroMemory(sendOverlap->buffer, MAX_BUFF_SIZE);
	sendOverlap->wsaBuf.buf = reinterpret_cast<CHAR*>(sendOverlap->buffer);
	sendOverlap->wsaBuf.len = packet[0];
	memcpy(sendOverlap->buffer, packet, packet[0]);

	Client *clientData = nullptr;
	gClientInfoSet->Search(index, &clientData);

	retval = WSASend(clientData->socket, &sendOverlap->wsaBuf, 1, NULL, 0, &sendOverlap->originalOverlap, NULL);
	if (0 != retval)
	{
		int errNo = WSAGetLastError();
		if (WSA_IO_PENDING != errNo)
			DisplayErrMsg("SendPacket :: WSASend", errNo);
		if (WSAECONNABORTED == errNo)
		{
			clientData->isConnect = false;
			closesocket(clientData->socket);
			
			// ToDo : 10054 error ó��
			// �� �÷��̾�� �÷��̾� ���� ���� �˸�
			Packet::Disconnect packet;
			packet.size = sizeof(Packet::Disconnect);
			packet.type = PacketType::Disconnect;
			packet.id = index;

			BroadcastingExceptIndex(index, reinterpret_cast<unsigned char*>(&packet));

			gClientInfoSet->Remove(index);
			DisplayDebugText("SendPacket :: Process error 10053.");
		}
	}
	return;
}

void BroadcastingExceptIndex(const int index, const unsigned char* packet)
{
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
			if (curr->data.isConnect && curr->id != index)
				SendPacket(curr->id, packet);
			pred = curr;
		}
	}
	return;
}

void Broadcasting(const unsigned char* packet)
{
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
			if (curr->data.isConnect)
				SendPacket(curr->id, packet);
			pred = curr;
		}
	}
	return;
}

void BroadcastingExceptIndex_With_UpdateRoomInfo(const int index)
{
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
			if (curr->data.isConnect && curr->id != index && curr->data.player.roomNo == 0)
				UpdateRoomInfo(curr->id);
			pred = curr;
		}
	}
	return;
}

void UpdateRoomInfo(const unsigned int id)
{
	Packet::Notify notifyPacket;
	notifyPacket.size = sizeof(Packet::Notify);
	notifyPacket.type = (BYTE)PacketType::Notify;
	// id�� ���� �ǹ� ����. SERVER���� �����ִ� ���̱� ����
	notifyPacket.id = NIL;
	notifyPacket.notice = Notice::UPDATE_ROOM;
	SendPacket(id, reinterpret_cast<unsigned char*>(&notifyPacket));
	Sleep(10);

	bool result = false, marked = false;
	RoomNode *pred, *curr;
	RoomNode *tail = gRoomInfoSet->GetTail();
	pred = gRoomInfoSet->GetHead();
	curr = pred;

	Packet::CreateRoom createRoomPacket;
	createRoomPacket.size = sizeof(Packet::CreateRoom);
	createRoomPacket.type = (BYTE)PacketType::CreateRoom;
	while (curr->GetNext() != tail)
	{
		curr = pred->GetNextWithMark(&marked);
		if (marked) continue;

		result = gRoomInfoSet->Contains(curr->id);
		if (result) {
			createRoomPacket.roomNo = curr->data.no;
			createRoomPacket.chiefNo = curr->data.chiefID;
			createRoomPacket.partner_1_ID = curr->data.partner_1_ID;
			createRoomPacket.partner_2_ID = curr->data.partner_2_ID;
			createRoomPacket.partner_3_ID = curr->data.partner_3_ID;

			SendPacket(id, reinterpret_cast<unsigned char*>(&createRoomPacket));
			
			// SendPacket�� ��Ȱ�� ó���ϱ� ���� delay
			Sleep(10);
			//std::cout << "Update Room Info : Send class #" << createRoomPacket.roomNo << " data to."<< id << std::endl;
			pred = curr;
		}
	}

	return;
}