#pragma once

#include "stdafx.h"
#include "defaultInit.h"

template <typename T>
class TBB_HASH_MAP
{
public:
	TBB_HASH_MAP() { }

	bool Insert(int key) {
		tbb::concurrent_hash_map<int, T>::accessor acc;
		return this->m_map.insert(acc,key);
	}

	bool Remove(int key) {
		return this->m_map.erase(key);
	}

	int Size() {
		return this->m_map.size();
	}

	bool Contains(int key) {
		tbb::concurrent_hash_map<int, T>::const_accessor con_acc;
		if (this->m_map.find(con_acc, key))
			return true;
		else
			return false;
	}

	void Clear() {
		this->m_map.clear();
		return;
	}

	T *GetData(int key) {
		tbb::concurrent_hash_map<int, T>::accessor acc;
		if (this->m_map.find(acc, key)) {
			return (T*)(&acc->second);
		}
		else
			return nullptr;
	}

public:
	tbb::concurrent_hash_map<int, T> m_map;
};

extern TBB_HASH_MAP<Client> gClientInfoMAP;