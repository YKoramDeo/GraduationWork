#pragma once

// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ 
//
//						iocpNetwork_H
//
//		IOCP�� ����ϱ� ���ؼ� �⺻������ �ʿ�� �ϴ� 
//		����ü�� �Լ��� ���� �� ������ ��
//
// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------

#include "protocol.h"
#include "defaultInit.h"

struct OverlapEx
{
	WSAOVERLAPPED	originalOverlap;			// ������ Overlapped ����ü
	WSABUF			wsaBuf;						// ������ WSABUF
	int				operation;					// ���� Send or Recv ������ �����ϴ� ������ ���θ� �����ϴ� ����
	unsigned char	buffer[MAX_BUFF_SIZE];		// IOCP Send / Recv Buffer
	int				packetSize;					// ���� packet�� ���� �����ϴ� ����.
};

struct Client
{
	int				id;
	bool			isConnect;
	SOCKET			socket;
	Player			player;
	OverlapEx		recvOverlap;
	unsigned char	packetBuf[MAX_BUFF_SIZE];	// Recv�Ǵ� ��Ŷ�� �����Ǵ� Buffer / Send ������ ������� �����Ƿ� Ȯ�� ����ü�� ���Ե��� ����.
	int				previousDataSize;			// ������ ���� ���� �����ϴ� ���� / Send ������ ������� �����Ƿ� Ȯ�� ����ü�� ���Ե��� ����.
};

extern bool gShutdown;
extern HANDLE ghIOCP;
extern Client gClientsList[MAX_USER];

void DisplayErrMsg(char*, int);
void DisplayDebugText(std::string);

void InitializeServer(void);
void StopServer(void);
void WorkerThreadFunc(void);
void AcceptThreadFunc(void);
void SendPacket(const int, const unsigned char*);
extern void ProcessPacket(int, unsigned char*);