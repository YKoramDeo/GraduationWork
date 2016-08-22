#include "stdafx.h"
#include "protocol.h"
#include "defaultInit.h"
#include "iocpNetwork.h"
#include "processRoutine.h"
#include "lock-free-SET_RoomInfo.h"
#include "tbb_hash_map.h"

bool gShutdown = false;
HANDLE ghIOCP;

RoomList *gRoomInfoSet;
RoomNode *gRoomInfo_DelList = nullptr;
std::mutex gRoomInfo_DelList_Lock;

TBB_HASH_MAP<Client> gClientInfoMAP;

int main(int argc, char *argv[])
{
	std::vector<std::thread*> workerThreads;
	std::thread acceptThread;

	gRoomInfoSet = new RoomList();
	gRoomInfo_DelList = new RoomNode(MIN_INT);

	gRoomInfoSet->Initialize();
	InitializeServer();

	acceptThread = std::thread(AcceptThreadFunc);
	for (int i = 0; i < NUM_THREADS; ++i)
		workerThreads.push_back(new std::thread(WorkerThreadFunc));

	while (!gShutdown) Sleep(1000);
	//  ������ ���� Main Thread�� 1�ʸ��� ���� �Ͽ�
	// Processor�� Main Thread�� �����ϴ� ���� �ִ��� �����ϵ��� �Ѵ�.

	acceptThread.join();
	for (auto t : workerThreads)
	{
		t->join();
		delete t;
	}

	StopServer();
	gClientInfoMAP.Clear();

	gRoomInfoSet->Rearrangement();
	while (0 != gRoomInfo_DelList->next)
	{
		RoomNode *temp = gRoomInfo_DelList;
		gRoomInfo_DelList = gRoomInfo_DelList->GetNext();
		delete temp;
	}

	delete gRoomInfo_DelList;
	delete gRoomInfoSet;

	return 0;
}