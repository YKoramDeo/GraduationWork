#include "iocpNetwork.h"

void DisplayErrMsg(char* msg, int errNo)
{
	WCHAR *lpMsgBuf;
	// FormatMessage() : 오류코드에 대응하는 오류 메시지를 얻을 수 있다.
	// msdn - The function finds the message definition in a message table resource vased on a message identifier and a language identifier.
	//		The funtion copies the formatted message text to an output buffer, processing ant embadded insert sequences if requested.

	// FORMAT_MESSAGE_ALLOCATE_BUFFER			: 오류 메시지를 저장할 공간을 함수가 알아서 할당한다는 의미
	// FORMAT_MESSAGE_FROM_SYSTEN				: OS로부터 오류 메시지를 가져온다는 의미

	// MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT)	: 사용자가 제어판에서 설정한 기본언어로 오류메시지 얻을 수 있음
	// lpMsgBuf 는 오류 메시지를 저장할 공간인데 함수가 알아서 할당, 
	// 오류 메시지 사용을 마치면 LocalFree() 함수를 이용해 할당한 메모리 반환 필수
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
	// WSAStartup() :  프로그램에서 사용할 췬속 버전을 요청함으로써 윈속 라이브러리(WS2_32.DLL)를 초기화 하는 역할
	// msdn - The WSAStartup function initiates use of Winsock DLL bt a process

	// MAKEWORD(2,2)	: 프로그램이 요구하는 최상위 윈속 버전, 하위 8비트에 주 버전, 상위 8비트에 부 버전을 넣어서 전달
	//					: 만일 윈속 3.2 버전을 사용을 요청한다면 0x0203 또는 MAKEWORD(3,2)를 사용
	// wsadata			: 윈도우 운영체제가 제공하는 윈속 구현에 관한 정보(프로그램이 실제로 사용하게 될 윈속 버전,
	//					: 시스템이 지원하는 윈속 최상위 버전 등)를 얻을 수 있음. 하지만 실제로 이런 정보 사용 X
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		DisplayDebugText("InitializeServer :: WSAStartup Fail !!");
		exit(1);
	}

	// Create IOCP kernel object
	// if last argument is 0, Use kernel object by core count
	// CreateIoCompletionPort() : 입출력 완료 포트를 새로 생성.
	//							: 입출력 롼료 포트란 비동기 입출력 결과와 이 결과를 처리할 스레드에 관한 정보를 담고 있는 구조
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
	WSACleanup();
	DisplayDebugText("Stop Server :: Called!!");
	return;
}

void WorkerThreadFunc(void)
{
	DWORD ioSize, key;
	OverlapEx *overlap;
	BOOL result;

	while (true)
	{
		result = GetQueuedCompletionStatus(ghIOCP, &ioSize, reinterpret_cast<PULONG_PTR>(&key), reinterpret_cast<LPOVERLAPPED*>(&overlap), INFINITE);

		if (false == result || 0 == ioSize)
		{
			if (false == result)
				DisplayErrMsg("WorkerThread :: GetQueuedCompletionStatus Fail !!", GetLastError());

			if (WSAENOTSOCK == GetLastError() || ERROR_NETNAME_DELETED == GetLastError() || 0 == ioSize)
			{
				gClientInfoMAP.GetData(key)->isConnect = false;
				closesocket(gClientInfoMAP.GetData(key)->socket);

				// ToDo : 10054 error 처리
				// 각 플레이어에게 플레이어 접속 종료 알림
				Packet::Disconnect packet;
				packet.size = sizeof(Packet::Disconnect);
				packet.type = PacketType::Disconnect;
				packet.id = key;

				BroadcastingExceptIndex(key, reinterpret_cast<unsigned char*>(&packet));
				gClientInfoMAP.Remove(key);

				std::string debugText = "WorkerThread :: Disconnect " + std::to_string(key) + " client. :(";
				DisplayDebugText(debugText);
			}
			continue;
		}
		if (OP_RECV == overlap->operation)
		{
			unsigned char *buf_ptr = overlap->buffer;
			int remained = ioSize;

			while (0 < remained)
			{
				if (0 == gClientInfoMAP.GetData(key)->recvOverlap.packetSize)
					gClientInfoMAP.GetData(key)->recvOverlap.packetSize = buf_ptr[0];
				int required = gClientInfoMAP.GetData(key)->recvOverlap.packetSize - gClientInfoMAP.GetData(key)->previousDataSize;
				// 패킷을 완성 시킬 수 있는가? 없는가?
				if (remained >= required)
				{
					// 완성을 시킬 수 있는 상황이면
					// 패킷을 어떠한 공간에다가 고이 모셔와야 한다.
					// 그래서 패킷을 완성시키는 저장공간이 별도로 있어야한다. 
					// 데이터가 패킷단위로 오는 것이 아니기 때문에 패킷단위로 처리하고 남은데이터는 그 별도의 공간에 저장해야
					// 다음의 온 데이터가 온전하지 못한 채로 오게되면 별도의 공간에 집어넣고 하나의 패킷으로 마저 만들어 주어야 한다.
					memcpy(gClientInfoMAP.GetData(key)->packetBuf + gClientInfoMAP.GetData(key)->previousDataSize, buf_ptr, required);
					// +하는 이유는 지난번에 받은 데이터 이후에 저장을 해야하기 때문에 그 시작위치로 옮겨줌.
					ProcessPacket(key, gClientInfoMAP.GetData(key)->packetBuf);
					// Packet 처리
					remained -= required;
					// 날아 있는 것은 필요한 것을 제외하고
					buf_ptr += required;
					// ???
					gClientInfoMAP.GetData(key)->recvOverlap.packetSize = 0;
				}
				else
				{
					// 패킷을 완성 시킬 수 없는 크기이다.
					memcpy(gClientInfoMAP.GetData(key)->packetBuf + gClientInfoMAP.GetData(key)->previousDataSize, buf_ptr, remained);
					// buf_ptr의 모든 것을 packet에 저장한다.
					gClientInfoMAP.GetData(key)->previousDataSize += remained;
					// 이전의 데이터가 남아있는 사이즈만큼 늘어났다.
					remained = 0;
					// 현재 recv의 남아있는 size는 이미 저장을 해두었으므로 초기화
				}
			}
			DWORD flags = 0;
			WSARecv(gClientInfoMAP.GetData(key)->socket,
				&gClientInfoMAP.GetData(key)->recvOverlap.wsaBuf,
				1, NULL, &flags,
				reinterpret_cast<LPWSAOVERLAPPED>(&gClientInfoMAP.GetData(key)->recvOverlap),
				NULL);
		}
		else if (OP_SEND == overlap->operation)
		{
			// ioSize하고 실제 보낸 크기 비교 후 소켓 접속 끊기
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

	// bind() : 소켓의 지역 IP주소와 지역 포트 번호를 결정, C++11의 bind라는 함수와 혼동 때문에::범위확인 연산자와 같이 사용.
	// msdn - The bind functio associates a local address with a socket

	// listenAddr	: 소켓 주소 구조체를 지역 IP주소와 지역 포트 번호로 초기화하여 전달한다.
	// AF_INET		: 인터넷 주초 체계(IPv4)를 사용한다는 의미
	// INADDR_ANY	: 서버의 지역 IP주소를 설정하는데 있어 다음과 같이 설정을 하면 서버가 IP주소를 2 개 이상 보유한 경우 
	//				(multihomed host라고 부름), 클라이언트가 어느 IP 주소로 접속하든 받아들일 수 있음
	// bind 함수의 2번째 인자는 항상 (SOCKADDR*)형으로 반환해야 함
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
	// listen() : 소켓의 TCP 포트 상태를 LISTENING으로 바꾼다. 이는 클라이언트 접속을 받아들일 수 있는 상태가 됨을 의미
	// msdn - The listen function places a socket in a state in which it is listening for an incomming connection.

	// SOMAXCONN : 클라이언트의 접속 정보는 연결 큐에 저장되는데, backlog는 이 연결 큐의 길이를 나타냄.
	//			SOMAXCONN의 의미는 하부 프로토콜에서 지원 가능한 최댓값을 사용
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

		// 접속한 클라이언트의 새 ID

		// accept() : 접속한 클라이언트와 통신할 수 있도록 새로운 소켓을 생성해서 리턴 또한 접속한 클라이언트의 주소정보도 알려줌
		//			: 서버입장에서는 원격 IP주소와 원격 포트번호, 클라이언트 입장에서는 지역 IP 주소와 지역 포트 번호
		// WSAAccept() : ?
		// msdn - The WSAAccept function conditionally accepts a connection based on the return value of a condition,
		//		provides quality of service flow specifications, and allows the transfer of connection data.
		SOCKET newClientSocket = WSAAccept(acceptSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrSize, NULL, NULL);
		if (INVALID_SOCKET == newClientSocket)
		{
			DisplayErrMsg("AcceptThread :: WSAAccept Fail !!", WSAGetLastError());
			break;
		}

		// 접속한 클라이언트의 새로운 ID 부여하는 구간 
		newID++;

		// Accept를 받은 이후에 등록을 해주어야 함
		HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(newClientSocket), ghIOCP, newID, 0);
		if (NULL == result)
		{
			DisplayErrMsg("AcceptThread :: CreateIoCompletionPort Fail !!", WSAGetLastError());
			closesocket(newClientSocket);
			continue;
		}

		gClientInfoMAP.Insert(newID);
		gClientInfoMAP.GetData(newID)->isConnect = true;
		gClientInfoMAP.GetData(newID)->socket = newClientSocket;
		memset(&gClientInfoMAP.GetData(newID)->recvOverlap.originalOverlap, 0, sizeof(WSAOVERLAPPED));
		ZeroMemory(gClientInfoMAP.GetData(newID)->recvOverlap.buffer, MAX_BUFF_SIZE);
		gClientInfoMAP.GetData(newID)->recvOverlap.wsaBuf.buf = reinterpret_cast<CHAR*>(gClientInfoMAP.GetData(newID)->recvOverlap.buffer);
		gClientInfoMAP.GetData(newID)->recvOverlap.wsaBuf.len = MAX_BUFF_SIZE;
		gClientInfoMAP.GetData(newID)->recvOverlap.operation = OP_RECV;
		gClientInfoMAP.GetData(newID)->recvOverlap.packetSize = 0;
		ZeroMemory(gClientInfoMAP.GetData(newID)->packetBuf, MAX_BUFF_SIZE);
		gClientInfoMAP.GetData(newID)->previousDataSize = 0;
		gClientInfoMAP.GetData(newID)->player.roomNo = 0;
		gClientInfoMAP.GetData(newID)->player.pos.x = DEFAULT_POS_X;
		gClientInfoMAP.GetData(newID)->player.pos.y = DEFAULT_POS_Y;
		gClientInfoMAP.GetData(newID)->player.pos.z = DEFAULT_POS_Z;

		// 새로운 클라이언트 접속 알림
		Packet::SetID clientSetIDPacket;
		clientSetIDPacket.size = sizeof(Packet::SetID);
		clientSetIDPacket.type = (BYTE)PacketType::SetID;
		clientSetIDPacket.id = newID;
		SendPacket(newID, reinterpret_cast<unsigned char*>(&clientSetIDPacket));

		// 새로운 소켓 Recv 수행
		DWORD flags = 0;
		// WSARecv() : 5번째, 6번째 두 인자를 NULL 값으로 사용하면 recv() 함수처럼 동기 함수로 동작
		retval = WSARecv(newClientSocket, &gClientInfoMAP.GetData(newID)->recvOverlap.wsaBuf, 1, NULL, &flags, &gClientInfoMAP.GetData(newID)->recvOverlap.originalOverlap, NULL);
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
	std::cout << "Enter Send Packet!" << std::endl;
	int retval = 0;
	OverlapEx *sendOverlap = new OverlapEx;
	memset(sendOverlap, 0, sizeof(OverlapEx));
	sendOverlap->operation = OP_SEND;
	memset(&sendOverlap->originalOverlap, 0, sizeof(WSAOVERLAPPED));
	ZeroMemory(sendOverlap->buffer, MAX_BUFF_SIZE);
	sendOverlap->wsaBuf.buf = reinterpret_cast<CHAR*>(sendOverlap->buffer);
	sendOverlap->wsaBuf.len = packet[0];
	memcpy(sendOverlap->buffer, packet, packet[0]);

	retval = WSASend(gClientInfoMAP.GetData(index)->socket, &sendOverlap->wsaBuf, 1, NULL, 0, &sendOverlap->originalOverlap, NULL);
	if (0 != retval)
	{
		int errNo = WSAGetLastError();
		if (WSA_IO_PENDING != errNo)
			DisplayErrMsg("SendPacket :: WSASend", errNo);
		if (WSAECONNABORTED == errNo)
		{
			gClientInfoMAP.GetData(index)->isConnect = false;
			closesocket(gClientInfoMAP.GetData(index)->socket);

			// ToDo : 10054 error 처리
			// 각 플레이어에게 플레이어 접속 종료 알림
			Packet::Disconnect packet;
			packet.size = sizeof(Packet::Disconnect);
			packet.type = PacketType::Disconnect;
			packet.id = index;

			BroadcastingExceptIndex(index, reinterpret_cast<unsigned char*>(&packet));

			gClientInfoMAP.Remove(index);
			DisplayDebugText("SendPacket :: Process error 10053.");
		}
	}
	return;
}

void BroadcastingExceptIndex(const int index, const unsigned char* packet)
{
	for (tbb::concurrent_hash_map<int, Client>::iterator i = gClientInfoMAP.m_map.begin(); i != gClientInfoMAP.m_map.end(); ++i)
	{
		int key = i->first;
		if (gClientInfoMAP.Contains(key))
		{
			if (gClientInfoMAP.GetData(key)->isConnect == false) continue;
			if (key == index) continue;
			SendPacket(key, packet);
		}
	}
	return;
}

void Broadcasting(const unsigned char* packet)
{
	for (tbb::concurrent_hash_map<int, Client>::iterator i = gClientInfoMAP.m_map.begin(); i != gClientInfoMAP.m_map.end(); ++i)
	{
		int key = i->first;
		if (gClientInfoMAP.Contains(key))
		{
			if (gClientInfoMAP.GetData(key)->isConnect == false) continue;
			SendPacket(key, packet);
		}
	}
	return;
}

void BroadcastingExceptIndex_With_UpdateRoomInfo(const int index)
{
	for (tbb::concurrent_hash_map<int, Client>::iterator i = gClientInfoMAP.m_map.begin(); i != gClientInfoMAP.m_map.end(); ++i)
	{
		int key = i->first;
		if (gClientInfoMAP.Contains(key))
		{
			if (gClientInfoMAP.GetData(key)->isConnect == false) continue;
			if (key == index) continue;
			if (gClientInfoMAP.GetData(key)->player.roomNo != 0) continue;
			UpdateRoomInfo(key);
		}
	}
	return;
}

void UpdateRoomInfo(const unsigned int id)
{
	std::cout << "Enter Update Room Info" << std::endl;
	Packet::Notify notifyPacket;
	notifyPacket.size = sizeof(Packet::Notify);
	notifyPacket.type = (BYTE)PacketType::Notify;
	// id는 별로 의미 없음. SERVER에서 보내주는 것이기 때문
	notifyPacket.id = NIL;
	notifyPacket.notice = Notice::UPDATE_ROOM;
	SendPacket(id, reinterpret_cast<unsigned char*>(&notifyPacket));

	std::cout << "Send Update Room Packet" << std::endl;

	Sleep(10);

	Packet::CreateRoom createRoomPacket;
	createRoomPacket.size = sizeof(Packet::CreateRoom);
	createRoomPacket.type = (BYTE)PacketType::CreateRoom;

	for (tbb::concurrent_hash_map<int, RoomInfo>::iterator i = gRoomInfoMAP.m_map.begin(); i != gRoomInfoMAP.m_map.end(); ++i)
	{
		int key = i->first;
		if (gRoomInfoMAP.Contains(key))
		{
			createRoomPacket.roomNo = gRoomInfoMAP.GetData(key)->no;
			createRoomPacket.chiefNo = gRoomInfoMAP.GetData(key)->chiefID;
			createRoomPacket.partner_1_ID = gRoomInfoMAP.GetData(key)->partner_1_ID;
			createRoomPacket.partner_2_ID = gRoomInfoMAP.GetData(key)->partner_2_ID;
			createRoomPacket.partner_3_ID = gRoomInfoMAP.GetData(key)->partner_3_ID;

			SendPacket(id, reinterpret_cast<unsigned char*>(&createRoomPacket));
			Sleep(10);
		}
	}

	return;
}