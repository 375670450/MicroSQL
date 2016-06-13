#pragma once

/*
	1. IndexHandle���ڲ���(��ѯ, ����, ɾ��)һ������, ���������ݿ���õ�������ͬ, ��ǰ���ڲ�ʵ��ҲΪB+��
	2. ÿ�����Ĵ�СΪUtils::PAGESIZE, ��ŵ�Key
	3. һ���ļ�����һ���������ڴ�, ָ���ӽڵ��ָ��������ļ��е�ƫ��
	
*/

#include "Utils.hpp"
#include "RecordIdentifier.hpp"

struct IndexHeader {				// the information of every index
	AttrType attrType;
	
	size_t attrLength;

};

struct NodeHeader {

	char identifyChar;				// 'I' for internal node, 'L' for leaf node

	size_t KeysCount;

	PageNum parent;

};

struct InternalNode {

	NodeHeader header;

	VoidPtr keys;				// the array of keys

	PageNumPtr childs;		// the array of child nodes

};

struct LeafNode {

	NodeHeader header;

	VoidPtr keys;		// the array of keys

	RecordIdentifierPtr rids;		// the array of record identifiers (value)

	PageNum prevLeaf;

	PageNum nextLeaf;

};

class IndexHandle {
public:

	using Comparator = int (*) ( void *, void *, size_t);

	IndexHandle ( );

	~IndexHandle ( );
	
	RETCODE InsertEntry (void *pData, const RecordIdentifier & rid);  // Insert new index entry

	RETCODE DeleteEntry (void *pData, const RecordIdentifier & rid);  // Delete index entry
	
	RETCODE ForcePages ( );                             // Copy index to disk

private:

	/*
		B+Tree Functions
	*/

	/*
		B+Tree Members
	*/

	PageNum rootPage;

	/*
		IndexHandle Members
	*/

	IndexHeader _header;

	BufferManagerPtr _bufMgr;
	
	bool _headerModified;

	bool _isOpenHandle;

	

};

IndexHandle::IndexHandle ( ) {
}

IndexHandle::~IndexHandle ( ) {
}
