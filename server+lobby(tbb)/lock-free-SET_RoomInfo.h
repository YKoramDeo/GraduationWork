#pragma once

#include "stdafx.h"
#include "defaultInit.h"

typedef RoomInfo Data;

class RoomNode
{
public:
	int id;
	RoomInfo data;
	int next;

	RoomNode() { next = 0; }
	RoomNode(int value) {
		next = 0;
		this->id = value;
		data.no = value;
		data.chiefID = NIL;
		data.partner_1_ID = NIL;
		data.partner_2_ID = NIL;
		data.partner_3_ID = NIL;
		data.partner_1_ready = false;
		data.partner_2_ready = false;
		data.partner_3_ready = false;
	}

	RoomNode(Data data) {
		next = 0;
		this->id = data.no;
		this->data.no = data.no;
		this->data.chiefID = data.chiefID;
		this->data.partner_1_ID = NIL;
		this->data.partner_2_ID = NIL;
		this->data.partner_3_ID = NIL;
		this->data.partner_1_ready = false;
		this->data.partner_2_ready = false;
		this->data.partner_3_ready = false;
	}

	~RoomNode() { }

	RoomNode *GetNext() {
		return reinterpret_cast<RoomNode *>(next & 0xFFFFFFFE);
	}

	RoomNode *GetNextWithMark(bool *removed) {
		*removed = (next & 1) == 1;
		return reinterpret_cast<RoomNode *>(next & 0xFFFFFFFE);
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

	bool CAS(RoomNode *old_addr, RoomNode *new_addr, bool old_mark, bool new_mark) {
		int old_value = reinterpret_cast<int>(old_addr);
		if (true == old_mark) old_value = old_value | 1;
		else old_value = old_value & 0xFFFFFFFE;

		int new_value = reinterpret_cast<int>(new_addr);
		if (new_mark) new_value = new_value | 1;
		else new_value = new_value & 0xFFFFFFFE;

		return CAS(old_value, new_value);
	}

	bool AttemptMark(RoomNode* node, bool newMark) {
		int old_value = reinterpret_cast<int>(node);
		int new_value = old_value;
		if (newMark) new_value = new_value | 0x01;
		else new_value = new_value & 0xFFFFFFFE;
		return CAS(old_value, new_value);
	}

	void SetNext(RoomNode *new_next) {
		next = reinterpret_cast<int>(new_next);
		return;
	}
};

extern RoomNode *gRoomInfo_DelList;
extern std::mutex gRoomInfo_DelList_Lock;

class RoomList
{
public:
	RoomList()
	{
		mHead.id = MAX_INT;
		mTail.id = MIN_INT;
		mHead.SetNext(&mTail);
		mIndex = 0;
	}

	~RoomList() { Initialize(); }

	void Initialize()
	{
		RoomNode *ptr = nullptr;
		RoomNode *temp_head = mHead.GetNext();
		RoomNode *temp_tail = &mTail;
		while (mHead.GetNext() != &mTail) {
			ptr = mHead.GetNext();
			mHead.SetNext(ptr->GetNext());
			delete ptr;
		}
		return;
	}

	void CheckElement(void)
	{
		RoomNode *curr;

		curr = &mHead;
		std::cout << "data : ";
		while (curr != 0)
		{
			std::cout << curr->id << " ";
			curr = curr->GetNext();
		}
		std::cout << std::endl;

		return;
	}

	bool Add(int id, Data data)
	{
		RoomNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, id);
			if (curr->id == id)
				return false;

			RoomNode* newNode = new RoomNode(data);
			newNode->SetNext(curr);
			if (pred->CAS(curr, newNode, false, false))
			{
				mIndex++;
				return true;
			}
			delete newNode;
		}
	}

	bool Add(int id)
	{
		RoomNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, id);
			if (curr->id == id)
				return false;

			RoomNode* newNode = new RoomNode(id);
			newNode->SetNext(curr);
			if (pred->CAS(curr, newNode, false, false))
			{
				mIndex++;
				return true;
			}
			delete newNode;
		}
	}

	bool Remove(int id)
	{
		RoomNode *pred, *curr;

		while (true)
		{
			Find(&pred, &curr, id);

			if (curr->id != id)
				return false;
			RoomNode *succ = curr->GetNext();
			if (!curr->AttemptMark(succ, true)) continue;
			//pred->CAS(curr, succ, false, false);
			Rearrangement();
			return true;
		}
	}

	bool Contains(int id)
	{
		RoomNode *succ;
		RoomNode *curr = &mHead;
		bool marked = false;
		while (curr->id < id)
		{
			curr = curr->GetNext();
			succ = curr->GetNextWithMark(&marked);
		}

		return (id == curr->id) && (!marked);
	}

	bool Update(int id, RoomInfo roomInfo)
	{
		RoomNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, id);
			if (curr->id != id)
				return false;
			else
			{
				return true;
			}
		}
	}

	bool JoinClient(const int roomNo, const int clientID)
	{
		RoomNode *pred, *curr;

		while (true) {
			Find(&pred, &curr, roomNo);
			if (curr->id != roomNo)
				return false;
			else
			{
				if (curr->data.partner_1_ID == NIL) {
					curr->data.partner_1_ID = clientID;
					return true;
				}
				else if (curr->data.partner_2_ID == NIL) {
					curr->data.partner_2_ID = clientID;
					return true;
				}
				else if (curr->data.partner_3_ID == NIL) {
					curr->data.partner_3_ID = clientID;
					return true;
				}
				else
					return false;
			}
		}
	}

	void Rearrangement()
	{
		RoomNode *pred, *curr;
		Find(&pred, &curr, MIN_INT);
		return;
	}

	bool Search(int index, RoomInfo **ref)
	{
		RoomNode *pred, *curr;

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

	RoomNode *GetHead()
	{
		return &mHead;
	}

	RoomNode *GetTail()
	{
		return &mTail;
	}

	int GetIndex()
	{
		return mIndex;
	}

private:
	RoomNode mHead, mTail;
	unsigned int mIndex;

	void Find(RoomNode **pred, RoomNode **curr, int id)
	{
		bool marked = false;
		bool snip = false;
		RoomNode* succ;

	RETRY:
		*pred = &mHead;
		*curr = (*pred)->GetNext();

		while (true) {
			succ = (*curr)->GetNextWithMark(&marked);
			while (marked)
			{
				snip = (*pred)->CAS((*curr), succ, false, false);
				if (!snip) goto RETRY;

				gRoomInfo_DelList_Lock.lock();
				(*curr)->next = reinterpret_cast<int>(gRoomInfo_DelList);
				gRoomInfo_DelList = *curr;
				gRoomInfo_DelList_Lock.unlock();

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
