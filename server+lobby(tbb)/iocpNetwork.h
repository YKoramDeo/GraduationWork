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
#include "tbb_hash_map.h"

/*
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
*/
// �ش�Ǵ� ������ lock-free SET�� ����ȭ�ϰ� ���� defaultInit�κ����� ����
// ���⿡ �ּ� ó���ؼ� ���ܵδ� ������ �ٸ� ���α׷����� ���� �� ������ �� �ֵ��� ���ܵ�

extern bool gShutdown;
extern HANDLE ghIOCP;

void DisplayErrMsg(char*, int);
void DisplayDebugText(std::string);

void InitializeServer(void);
void StopServer(void);
void WorkerThreadFunc(void);
void AcceptThreadFunc(void);
void SendPacket(const int, const unsigned char*);

extern void ProcessPacket(int, unsigned char*);
extern bool BeCompeletedSendPacket(BYTE, BYTE);

extern void BroadcastingExceptIndex(const int, const unsigned char*);
extern void Broadcasting(const unsigned char*);
extern void BroadcastingExceptIndex_With_UpdateRoomInfo(const int);
extern void UpdateRoomInfo(const unsigned int);