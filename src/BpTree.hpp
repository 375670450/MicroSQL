#pragma once

/*
	1. ÿ�����ÿ����������һ��B+��
	2. B+����Ҷ�ڵ���ڲ����ʹ�ò�ͬ�����ݽṹ, 



*/

#include "Utils.hpp"
#include "RecordIdentifier.hpp"

#include <memory>
#include <algorithm>


//template<typename KeyType, typename ValueType, typename Comparator = std::less<KeyType>>
class BpTree {
public:

	using Comparator = int (*)( void*, void*, size_t);			// The type of compare function

	BpTree ( );

	BpTree (const BpTree &);
	
	~BpTree ( );

private:



};

using BpTreePtr = shared_ptr<BpTree>;


BpTree::BpTree ( ) {

}

inline BpTree::BpTree (const BpTree &) {
}

BpTree::~BpTree ( ) {

}
