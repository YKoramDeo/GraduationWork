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
		std::cout << "InitializeServer::WSAStartup Fail !!" << std::endl;
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
		std::cout << "InitializeServer::CreateIoCompletionPort Fail !!" << std::endl;
		exit(1);
	}

	return;
}

void StopServer(void)
{
	for (auto client : gClientsList)
	{
		if (client.isConnect)
			closesocket(client.socket);
	}

	WSACleanup();
	std::cout << "Stop Server::Called!!" << std::endl;
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
				DisplayErrMsg("WorkerThreadFunc::GetQueuedCompletionStatus Fail !!", GetLastError());

			if (WSAENOTSOCK == GetLastError() || ERROR_NETNAME_DELETED == GetLastError() || 0 == ioSize)
			{
				closesocket(gClientsList[key].socket);

				// ToDo : 10054 error ó��
				// �� �÷��̾�� �÷��̾� ���� ���� �˸�
				Packet::Disconnect packet;
				packet.size = sizeof(Packet::Disconnect);
				packet.type = PacketType::Disconnect;
				packet.id = key;

				for (auto ci = 0; ci < MAX_USER; ++ci)
				{
					if (!gClientsList[ci].isConnect) continue;
					if (ci == key) continue;
					SendPacket(ci, reinterpret_cast<unsigned char*>(&packet));
				}

				gClientsList[key].isConnect = false;

				std::cout << "WorkerThreadFunc	::Disconnect " << key << " client. :(" << std::endl;
			}
			continue;
		}
		if (OP_RECV == overlap->operation)
		{
			std::cout << "WorkerThreadFunc	:: Send Packet io Size : " << ioSize << std::endl;

			unsigned char *buf_ptr = overlap->buffer;
			int remained = ioSize;
			while (0 < remained)
			{
				if (0 == gClientsList[key].recvOverlap.packetSize)
					gClientsList[key].recvOverlap.packetSize = buf_ptr[0];
				int required = gClientsList[key].recvOverlap.packetSize - gClientsList[key].previousDataSize;
				// ��Ŷ�� �ϼ� ��ų �� �ִ°�? ���°�?
				if (remained >= required)
				{
					// �ϼ��� ��ų �� �ִ� ��Ȳ�̸�
					// ��Ŷ�� ��� �������ٰ� ���� ��ſ;� �Ѵ�.
					// �׷��� ��Ŷ�� �ϼ���Ű�� ��������� ������ �־���Ѵ�. 
					// �����Ͱ� ��Ŷ������ ���� ���� �ƴϱ� ������ ��Ŷ������ ó���ϰ� ���������ʹ� �� ������ ������ �����ؾ�
					// ������ �� �����Ͱ� �������� ���� ä�� ���ԵǸ� ������ ������ ����ְ� �ϳ��� ��Ŷ���� ���� ����� �־�� �Ѵ�.
					memcpy(gClientsList[key].packetBuf + gClientsList[key].previousDataSize, buf_ptr, required);
					// +�ϴ� ������ �������� ���� ������ ���Ŀ� ������ �ؾ��ϱ� ������ �� ������ġ�� �Ű���.
					ProcessPacket(key, gClientsList[key].packetBuf);
					// Packet ó��
					remained -= required;
					// ���� �ִ� ���� �ʿ��� ���� �����ϰ�
					buf_ptr += required;
					// ???
					gClientsList[key].recvOverlap.packetSize = 0;
				}
				else
				{
					// ��Ŷ�� �ϼ� ��ų �� ���� ũ���̴�.
					memcpy(gClientsList[key].packetBuf + gClientsList[key].previousDataSize, buf_ptr, remained);
					// buf_ptr�� ��� ���� packet�� �����Ѵ�.
					gClientsList[key].previousDataSize += remained;
					// ������ �����Ͱ� �����ִ� �����ŭ �þ��.
					remained = 0;
					// ���� recv�� �����ִ� size�� �̹� ������ �صξ����Ƿ� �ʱ�ȭ
				}
			}
			DWORD flags = 0;
			WSARecv(gClientsList[key].socket,
				&gClientsList[key].recvOverlap.wsaBuf,
				1, NULL, &flags,
				reinterpret_cast<LPWSAOVERLAPPED>(&gClientsList[key].recvOverlap),
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
				std::cout << "WorkerThreadFunc:: "<< key <<" client don't send " << packetType << "No. packet" << std::endl;
		}
		else
		{
			std::cout << "WorkerThreadFunc::Unknown Event on worker_thread" << std::endl;
			continue;
		}
	}
	return;
}

void AcceptThreadFunc(void)
{
	int retval = 0;
	struct sockaddr_in listenAddr;

	SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == acceptSocket)
	{
		DisplayErrMsg("AcceptThreadFunc::acceptSocket Fail !!", WSAGetLastError());
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
		DisplayErrMsg("AcceptThreadFunc::bind Fail !!", WSAGetLastError());
		exit(1);
	}
	// listen() : ������ TCP ��Ʈ ���¸� LISTENING���� �ٲ۴�. �̴� Ŭ���̾�Ʈ ������ �޾Ƶ��� �� �ִ� ���°� ���� �ǹ�
	// msdn - The listen function places a socket in a state in which it is listening for an incomming connection.

	// SOMAXCONN : Ŭ���̾�Ʈ�� ���� ������ ���� ť�� ����Ǵµ�, backlog�� �� ���� ť�� ���̸� ��Ÿ��.
	//			SOMAXCONN�� �ǹ̴� �Ϻ� �������ݿ��� ���� ������ �ִ��� ���
	listen(acceptSocket, SOMAXCONN);
	if (SOCKET_ERROR == retval)
	{
		DisplayErrMsg("AcceptThreadFunc::listen Fail !!", WSAGetLastError());
		exit(1);
	}

	while (true)
	{
		struct sockaddr_in clientAddr;
		int addrSize = sizeof(clientAddr);

		int newID = -1;
		// ������ Ŭ���̾�Ʈ�� �� ID

		// accept() : ������ Ŭ���̾�Ʈ�� ����� �� �ֵ��� ���ο� ������ �����ؼ� ���� ���� ������ Ŭ���̾�Ʈ�� �ּ������� �˷���
		//			: �������忡���� ���� IP�ּҿ� ���� ��Ʈ��ȣ, Ŭ���̾�Ʈ ���忡���� ���� IP �ּҿ� ���� ��Ʈ ��ȣ
		// WSAAccept() : ?
		// msdn - The WSAAccept function conditionally accepts a connection based on the return value of a condition,
		//		provides quality of service flow specifications, and allows the transfer of connection data.
		SOCKET newClientSocket = WSAAccept(acceptSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrSize, NULL, NULL);
		if (INVALID_SOCKET == newClientSocket)
		{
			DisplayErrMsg("AcceptThreadFunc::WSAAccept Fail !!", WSAGetLastError());
			break;
		}

		// ������ Ŭ���̾�Ʈ�� ���ο� ID �ο��ϴ� ���� 
		for (auto ci = 0; ci < MAX_USER; ++ci)
		{
			if (gClientsList[ci].isConnect == false)
			{
				newID = ci;
				break;
			}
		}

		if (-1 == newID)
		{
			std::cout << "AcceptThread::Maximum User Number fail !!" << std::endl;
			closesocket(newClientSocket);
			continue;
		}

		// Accept�� ���� ���Ŀ� ����� ���־�� ��
		HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(newClientSocket), ghIOCP, newID, 0);
		if (NULL == result)
		{
			DisplayErrMsg("AcceptThreadFunc::CreateIoCompletionPort Fail !!", WSAGetLastError());
			closesocket(newClientSocket);
			continue;
		}

		gClientsList[newID].id = newID;
		gClientsList[newID].isConnect = true;
		gClientsList[newID].socket = newClientSocket;
		memset(&gClientsList[newID].recvOverlap.originalOverlap, 0, sizeof(WSAOVERLAPPED));
		// WSARecv, WSASend�� �ϱ� �������� ������ Overlap ����ü�� �ʱ�ȭ�� ���־�� �Ѵ�.
		// �׷��� ������ ������ err_number 6�� "�ڵ��� �����ϴ�."��� ������ �����´�.
		ZeroMemory(gClientsList[newID].recvOverlap.buffer, MAX_BUFF_SIZE);
		gClientsList[newID].recvOverlap.wsaBuf.buf = reinterpret_cast<CHAR*>(gClientsList[newID].recvOverlap.buffer);
		gClientsList[newID].recvOverlap.wsaBuf.len = MAX_BUFF_SIZE;
		gClientsList[newID].recvOverlap.operation = OP_RECV;
		gClientsList[newID].recvOverlap.packetSize = 0;
		ZeroMemory(gClientsList[newID].packetBuf, MAX_BUFF_SIZE);
		gClientsList[newID].previousDataSize = 0;
		// newClient.player ���� ä���
		gClientsList[newID].player.pos.x = 263.0f;
		gClientsList[newID].player.pos.y = -14.0f;
		gClientsList[newID].player.pos.z = -2.0f;

		// ���ο� Ŭ���̾�Ʈ ���� �˸�
		Packet::SetID clientSetIDPacket;
		clientSetIDPacket.size = sizeof(Packet::SetID);
		clientSetIDPacket.type = (BYTE)PacketType::SetID;
		clientSetIDPacket.id = newID;
		SendPacket(newID, reinterpret_cast<unsigned char*>(&clientSetIDPacket));

		// ���ο� ���� Recv ����
		DWORD flags = 0;
		// WSARecv() : 5��°, 6��° �� ���ڸ� NULL ������ ����ϸ� recv() �Լ�ó�� ���� �Լ��� ����
		retval = WSARecv(newClientSocket, &gClientsList[newID].recvOverlap.wsaBuf, 1, NULL, &flags, &gClientsList[newID].recvOverlap.originalOverlap, NULL);
		if (0 != retval)
		{
			int errNo = WSAGetLastError();
			if (WSA_IO_PENDING != errNo)
			{
				DisplayErrMsg("AcceptThreadFunc::WSARecv", errNo);
			}
		}

		// Output
		std::cout << "AcceptThreadFunc	:: " << newID << " client Accept Success !!" << std::endl;
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
	retval = WSASend(gClientsList[index].socket, &sendOverlap->wsaBuf, 1, NULL, 0, &sendOverlap->originalOverlap, NULL);
	if (0 != retval)
	{
		int errNo = WSAGetLastError();
		if (WSA_IO_PENDING != errNo)
			DisplayErrMsg("SendPacket::WSASend", errNo);
		if (WSAECONNABORTED == errNo)
		{
			closesocket(gClientsList[index].socket);

			// ToDo : 10054 error ó��
			// �� �÷��̾�� �÷��̾� ���� ���� �˸�
			Packet::Disconnect packet;
			packet.size = sizeof(Packet::Disconnect);
			packet.type = PacketType::Disconnect;
			packet.id = index;

			for (auto ci = 0; ci < MAX_USER; ++ci)
			{
				if (!gClientsList[ci].isConnect) continue;
				if (ci == index) continue;
				SendPacket(ci, reinterpret_cast<unsigned char*>(&packet));
			}

			gClientsList[index].isConnect = false;
			DisplayDebugText("SendPacket::Process error 10053.");
		}
	}
	return;
}