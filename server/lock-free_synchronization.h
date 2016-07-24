#pragma once
#include "stdafx.h"
#include "defaultInit.h"

#define MAX_INT 0x80000000
#define MIN_INT 0x7FFFFFFF

/*
struct Data
{
	int id;
};
*/

typedef Client Data;

class LFNode
{
public:
	Data data;
	int next;
	
	LFNode() { next = 0; }
	LFNode(int value) {
		next = 0;
		data.id = value;
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
		data.player.pos.x = 0;
		data.player.pos.y = 0;
		data.player.pos.z = 0;
	}

	LFNode(Data data) {
		next = 0;
		this->data.id = data.id;
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
		this->data.player.pos.x = 0;
		this->data.player.pos.y = 0;
		this->data.player.pos.z = 0;
	}

	~LFNode() { }

	LFNode *GetNext() {
		return reinterpret_cast<LFNode*>(next & 0xFFFFFFFE);
	}

	LFNode *GetNextWithMark(bool *removed) {
		*removed = (next & 1) == 1;
		return reinterpret_cast<LFNode *>(next & 0xFFFFFFFE);
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

	bool CAS(LFNode *old_addr, LFNode *new_addr, bool old_mark, bool new_mark) {
		int old_value = reinterpret_cast<int>(old_addr);
		if (true == old_mark) old_value = old_value | 1;
		else old_value = old_value & 0xFFFFFFFE;

		int new_value = reinterpret_cast<int>(new_addr);
		if (new_mark) new_value = new_value | 1;
		else new_value = new_value & 0xFFFFFFFE;

		return CAS(old_value, new_value);
	}

	bool AttemptMark(LFNode* node, bool newMark) {
		int old_value = reinterpret_cast<int>(node);
		int new_value = old_value;
		if (newMark) new_value = new_value | 0x01;
		else new_value = new_value & 0xFFFFFFFE;
		return CAS(old_value, new_value);
	}

	void SetNext(LFNode *new_next) {
		next = reinterpret_cast<int>(new_next);
		return;
	}
};

extern LFNode *gClientInfo_DelList;
extern std::mutex gClientInfo_DelList_Lock;

class LFList
{
public:
	LFList()
	{
		mHead.data.id = MAX_INT;
		mTail.data.id = MIN_INT;
		mHead.SetNext(&mTail);
	}

	~LFList() { Initialize(); }

	void Initialize()
	{
		LFNode *ptr;
		LFNode *temp_head = mHead.GetNext();
		LFNode *temp_tail = &mTail;
		while (mHead.GetNext() != &mTail) {
			ptr = mHead.GetNext();
			mHead.SetNext(ptr->GetNext());
			delete ptr;
		}
		return;
	}

	void CheckElement(void)
	{
		LFNode *curr;

		curr = &mHead;
		std::cout << "data : ";
		while (curr != 0)
		{
			std::cout << curr->data.id << " ";
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

	bool Add(Data data)
	{
		LFNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, data.id);
			if (curr->data.id == data.id)
				return false;

			LFNode* newNode = new LFNode(data);
			newNode->SetNext(curr);
			if (pred->CAS(curr, newNode, false, false))
				return true;
			delete newNode;
		}
	}

	bool Add(int id)
	{
		LFNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, id);
			if (curr->data.id == id)
				return false;

			LFNode* newNode = new LFNode(id);
			newNode->SetNext(curr);
			if (pred->CAS(curr, newNode, false, false))
				return true;
			delete newNode;
		}
	}

	bool Remove(int id)
	{
		LFNode *pred, *curr;

		while (true)
		{
			Find(&pred, &curr, id);

			if (curr->data.id != id)
				return false;
			LFNode *succ = curr->GetNext();
			if (!curr->AttemptMark(succ, true)) continue;
			//pred->CAS(curr, succ, false, false);
			return true;
		}
	}

	bool Contains(int id)
	{
		LFNode *succ;
		LFNode *curr = &mHead;
		bool marked = false;
		while (curr->data.id < id)
		{
			curr = curr->GetNext();
			succ = curr->GetNextWithMark(&marked);
		}

		return (id == curr->data.id) && (!marked);
	}

	bool Update(int id)
	{
		LFNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, id);
			if (curr->data.id != id)
				return false;
			else
			{
				LFNode *succ = curr->GetNext();
				if (!curr->AttemptMark(succ, true)) continue;
				//pred->CAS(curr, succ, false, false);
				std::cout << "update complete!" << std::endl;
				return true;
			}
		}
	}

	void CleanElement()
	{
		LFNode *pred, *curr;
		Find(&pred, &curr, MIN_INT);
		return;
	}

	bool Search(int index, Client **ref)
	{
		LFNode *pred, *curr;
		
		while (true)
		{
			Find(&pred, &curr, index);

			if (curr->data.id != index)
				return false;
			
			*ref = &(curr->data);
			//pred->CAS(curr, succ, false, false);
			return true;
		}
	}

private:
	LFNode mHead, mTail;

	void Find(LFNode **pred, LFNode **curr, int id)
	{
		bool marked = false;
		bool snip = false;
		LFNode* succ;

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
			if ((*curr)->data.id >= id) return;
			(*pred) = (*curr);
			(*curr) = succ;
		}
		return;
	}
};

extern LFList *gClientInfoSet;
