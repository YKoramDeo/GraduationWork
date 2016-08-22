#pragma once
#include "stdafx.h"
#include "defaultInit.h"

class ClientNode
{
public:
	int id;
	Client data;
	int next;
	
	ClientNode() { next = 0; }
	ClientNode(int value) {
		next = 0;
		this->id = value;
		data.isConnect = true;
		data.socket = NULL;
		memset(&data.recvOverlap.originalOverlap, 0, sizeof(WSAOVERLAPPED));
		ZeroMemory(data.recvOverlap.buffer, MAX_BUFF_SIZE);
		data.recvOverlap.wsaBuf.buf = reinterpret_cast<CHAR*>(data.recvOverlap.buffer);
		data.recvOverlap.wsaBuf.len = MAX_BUFF_SIZE;
		data.recvOverlap.operation = OP_RECV;
		data.recvOverlap.packetSize = 0;
		ZeroMemory(data.packetBuf, MAX_BUFF_SIZE);
		data.previousDataSize = 0;
		data.player.roomNo = 0;
		data.player.pos.x = 0;
		data.player.pos.y = 0;
		data.player.pos.z = 0;
	}

	ClientNode(int id, Client data) {
		next = 0;
		this->id = id;
		this->data.isConnect = true;
		this->data.socket = NULL;
		memset(&this->data.recvOverlap.originalOverlap, 0, sizeof(WSAOVERLAPPED));
		ZeroMemory(this->data.recvOverlap.buffer, MAX_BUFF_SIZE);
		this->data.recvOverlap.wsaBuf.buf = reinterpret_cast<CHAR*>(this->data.recvOverlap.buffer);
		this->data.recvOverlap.wsaBuf.len = MAX_BUFF_SIZE;
		this->data.recvOverlap.operation = OP_RECV;
		this->data.recvOverlap.packetSize = 0;
		ZeroMemory(this->data.packetBuf, MAX_BUFF_SIZE);
		this->data.previousDataSize = 0;
		this->data.player.roomNo = 0;
		this->data.player.pos.x = 0;
		this->data.player.pos.y = 0;
		this->data.player.pos.z = 0;
	}

	~ClientNode() { }

	ClientNode *GetNext() {
		return reinterpret_cast<ClientNode*>(next & 0xFFFFFFFE);
	}

	ClientNode *GetNextWithMark(bool *removed) {
		*removed = (next & 1) == 1;
		return reinterpret_cast<ClientNode *>(next & 0xFFFFFFFE);
	}

	bool GetMark() {
		int marked = false;
		marked = next & 0x01;
		return (0x01 == marked) ? true : false;
	}

	bool CAS(int old_value, int new_value) {
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<std::atomic_int *>(&next), &old_value, new_value);
	}

	bool CAS(ClientNode *old_addr, ClientNode *new_addr, bool old_mark, bool new_mark) {
		int old_value = reinterpret_cast<int>(old_addr);
		if (true == old_mark) old_value = old_value | 1;
		else old_value = old_value & 0xFFFFFFFE;

		int new_value = reinterpret_cast<int>(new_addr);
		if (new_mark) new_value = new_value | 1;
		else new_value = new_value & 0xFFFFFFFE;

		return CAS(old_value, new_value);
	}

	bool AttemptMark(ClientNode* node, bool newMark) {
		int old_value = reinterpret_cast<int>(node);
		int new_value = old_value;
		if (newMark) new_value = new_value | 0x01;
		else new_value = new_value & 0xFFFFFFFE;
		return CAS(old_value, new_value);
	}

	void SetNext(ClientNode *new_next) {
		next = reinterpret_cast<int>(new_next);
		return;
	}
};

extern ClientNode *gClientInfo_DelList;
extern std::mutex gClientInfo_DelList_Lock;

class ClientList
{
public:
	ClientList()
	{
		mHead.id = MAX_INT;
		mTail.id = MIN_INT;
		mHead.SetNext(&mTail);
	}

	~ClientList() { Initialize(); }

	void Initialize()
	{
		ClientNode *ptr;
		ClientNode *temp_head = mHead.GetNext();
		ClientNode *temp_tail = &mTail;
		while (mHead.GetNext() != &mTail) {
			ptr = mHead.GetNext();
			mHead.SetNext(ptr->GetNext());
			if(socket != NULL) closesocket(ptr->data.socket);
			delete ptr;
		}
		return;
	}

	void CheckElement(void)
	{
		ClientNode *curr;

		curr = &mHead;
		std::cout << "data : ";
		while (curr != 0)
		{
			std::cout << curr->id << " ";
			curr = curr->GetNext();
		}
		std::cout << std::endl;

		curr = &mHead;
		std::cout << "next : ";
		while (curr != 0)
		{
			std::cout << curr->next << " ";
			curr = curr->GetNext();
		}
		std::cout << std::endl;

		curr = &mHead;
		std::cout << "mark : ";
		while (curr != 0)
		{
			std::cout << curr->GetMark() << " ";
			curr = curr->GetNext();
		}
		std::cout << std::endl;

		return;
	}

	bool Add(int id, Client data)
	{
		ClientNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, id);
			if (curr->id == id)
				return false;

			ClientNode* newNode = new ClientNode(id, data);
			newNode->SetNext(curr);
			if (pred->CAS(curr, newNode, false, false))
				return true;
			delete newNode;
		}
	}

	bool Add(int id)
	{
		ClientNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, id);
			if (curr->id == id)
				return false;

			ClientNode* newNode = new ClientNode(id);
			newNode->SetNext(curr);
			if (pred->CAS(curr, newNode, false, false))
				return true;
			delete newNode;
		}
	}

	bool Remove(int id)
	{
		ClientNode *pred, *curr;

		while (true)
		{
			Find(&pred, &curr, id);

			if (curr->id != id)
				return false;
			ClientNode *succ = curr->GetNext();
			if (!curr->AttemptMark(succ, true)) continue;
			//pred->CAS(curr, succ, false, false);
			Rearrangement();
			return true;
		}
	}

	bool Contains(int id)
	{
		ClientNode *succ;
		ClientNode *curr = &mHead;
		bool marked = false;
		while (curr->id < id)
		{
			curr = curr->GetNext();
			succ = curr->GetNextWithMark(&marked);
		}

		return (id == curr->id) && (!marked);
	}

	bool Update(int id, Player player)
	{
		ClientNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, id);
			if (curr->id != id)
				return false;
			else
			{
				curr->data.player.roomNo = player.roomNo;
				curr->data.player.pos.x = player.pos.x;
				curr->data.player.pos.y = player.pos.y;
				curr->data.player.pos.z = player.pos.z;
				return true;
			}
		}
	}

	void Rearrangement()
	{
		ClientNode *pred, *curr;
		Find(&pred, &curr, MIN_INT);
		return;
	}

	bool Search(int index, Client **ref)
	{
		ClientNode *pred, *curr;
		
		while (true)
		{
			Find(&pred, &curr, index);

			if (curr->id != index)
				return false;
			
			*ref = &(curr->data);
			//pred->CAS(curr, succ, false, false);
			return true;
		}
	}

	ClientNode *GetHead()
	{
		return &mHead;
	}

	ClientNode *GetTail()
	{
		return &mTail;
	}

private:
	ClientNode mHead, mTail;

	void Find(ClientNode **pred, ClientNode **curr, int id)
	{
		bool marked = false;
		bool snip = false;
		ClientNode* succ;

	RETRY:
		*pred = &mHead;
		*curr = (*pred)->GetNext();

		while (true) {
			succ = (*curr)->GetNextWithMark(&marked);
			while (marked)
			{
				snip = (*pred)->CAS((*curr), succ, false, false);
				if (!snip) goto RETRY;

				gClientInfo_DelList_Lock.lock();
				(*curr)->next = reinterpret_cast<int>(gClientInfo_DelList);
				gClientInfo_DelList = *curr;
				gClientInfo_DelList_Lock.unlock();

				*curr = succ;
				succ = (*curr)->GetNextWithMark(&marked);
			}
			if ((*curr)->id >= id) return;
			(*pred) = (*curr);
			(*curr) = succ;
		}
		return;
	}
};

extern ClientList *gClientInfoSet;
