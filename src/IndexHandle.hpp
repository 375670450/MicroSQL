#pragma once

/*
	1. IndexHandle���ڲ���(��ѯ, ����, ɾ��)һ������, ���������ݿ���õ�������ͬ, ��ǰ���ڲ�ʵ��ҲΪB+��
	2. ÿ�����Ĵ�СΪUtils::PAGESIZE, ��ŵ�Key
	3. һ���ļ�����һ���������ڴ�, ָ���ӽڵ��ָ��������ļ��е�ƫ��
	4. Ҷ�ڵ���ڲ���㶼ͳһ��һ��struct����
*/

#include "Utils.hpp"
#include "RecordIdentifier.hpp"
#include "BpTreeNode.hpp"

struct IndexHeader {				// the information of every index
	AttrType attrType;
	
	PageNum rootPage;

	size_t attrLength;

	size_t numMaxKeys;			// the order of tree

	size_t numPages;

	size_t height;



};

class IndexHandle {
	friend class IndexManager;

public:
	using Comparator = BpTreeNode::Comparator;

	//IndexHandle ( );
	IndexHandle (AttrType type, size_t attrlen);
	~IndexHandle ( );

	/*
	The file must be opened before any other operation
	*/
	RETCODE Open (BufferManagerPtr buf);

	RETCODE InsertEntry (void *pData, const RecordIdentifier & rid);  // Insert new index entry ( b+tree algorithm)

	RETCODE DeleteEntry (void *pData, const RecordIdentifier & rid);  // Delete index entry
	
	RETCODE ForcePages ( );                             // Copy all pages (the whole b+tree) to disk

	RETCODE ReadHeader ( );

	BpTreeNodePtr FetchNode (PageNum page) const;

	BpTreeNodePtr FetchNode (const RecordIdentifier &) const;

	BpTreeNodePtr FindLeaf (void * pData);

	bool IsValid ( ) const;

private:

	/*
		Get Info
	*/

	AttrType attrType ( ) const;

	size_t attrLen ( ) const;

	size_t numMaxKeys ( ) const;		// the order of tree

	size_t numPages ( ) const;

	size_t height ( ) const;

/* B+Tree Members */
	BpTreeNodePtr root;
	void * largestKey;

/*	IndexHandle Members */

	IndexHeader header;

	BufferManagerPtr bufMgr;
	
	bool headerModified;

	bool isOpenHandle;

	std::vector<BpTreeNodePtr> path;
	
	std::vector<PageNum> pathPage;

};

using IndexHandlePtr = shared_ptr<IndexHandle>;
//
//IndexHandle::IndexHandle ( ) {
//	largestKey = root = nullptr;
//	
//	header.height = 0;
//
//}

inline IndexHandle::IndexHandle (AttrType type, size_t attrlen) {
	//bufMgr = buf;
	header.attrType = type;
	header.attrLength = attrlen;
	header.height = 0;
	header.numPages = 0;
	header.numMaxKeys = ( Utils::PAGESIZE - sizeof (BpTreeNodeHeader) ) / ( sizeof (attrlen) + sizeof (RecordIdentifier) );

	headerModified = false;
	isOpenHandle = false;
	root = nullptr;

}

IndexHandle::~IndexHandle ( ) {

}

/*
The file must be opened before any other operation
*/
inline RETCODE IndexHandle::Open (BufferManagerPtr buf) {
	PagePtr pagePtr;
	DataPtr pData;

	if ( buf == nullptr )
		return RETCODE::INVALIDOPEN;

	bufMgr = buf;

	ReadHeader ( );

	isOpenHandle = true;
	headerModified = false;
	
	bool needInit = true;
	if ( header.height == 0 ) {		// is empty tree (without root)
		
		header.rootPage = 1;
		header.height = 1;

	}

	bufMgr->GetPage (header.rootPage, pagePtr);

	root = make_shared<BpTreeNode> (header.attrType, header.attrLength, pagePtr);

	return RETCODE::COMPLETE;
}

inline RETCODE IndexHandle::InsertEntry (void * pData, const RecordIdentifier & rid) {
	
	if ( pData == nullptr )
		return BADKEY;

	BpTreeNodePtr node = FindLeaf (pData);			// find the leaf node to insert
	BpTreeNodePtr newNode = nullptr;
	bool newLargest = false;

	assert (node != nullptr);

	size_t fitPos = node->FindKey (pData, rid);
	if ( fitPos != -1 )
		return RETCODE::ENTRYEXISTS;

	if ( node->GetNumKeys ( ) == 0 || node->comp (pData, largestKey) > 0 ) {

	}


	/*MarkDirty and UnLock*/
	//bufMgr->MarkDirty()
	header.numPages++;
	return RETCODE::COMPLETE;
}

inline RETCODE IndexHandle::ReadHeader ( ) {

	PagePtr pagePtr;
	DataPtr pData;
	RETCODE result;

	if ( result = bufMgr->GetPage (0, pagePtr) ) {
		Utils::PrintRetcode (result);
		return result;
	}
	
	if ( result = pagePtr->GetData (pData) ) {
		Utils::PrintRetcode (result);
		return result;
	}
	
	memcpy_s (reinterpret_cast< void* >( &header ), sizeof (IndexHeader), pData.get ( ), sizeof (IndexHeader));

	return RETCODE::COMPLETE;
}

inline BpTreeNodePtr IndexHandle::FetchNode (PageNum page) const {
	return FetchNode(RecordIdentifier{page, 0});
}

inline BpTreeNodePtr IndexHandle::FetchNode (const RecordIdentifier & rid) const {
	
	PageNum page;
	PagePtr pagePtr;
	RETCODE result;

	if ( result = rid.GetPageNum (page) ) {
		Utils::PrintRetcode (result);
		return nullptr;
	}

	if ( result = bufMgr->GetPage (page, pagePtr) ) {
		Utils::PrintRetcode (result);
		return nullptr;
	}

	return make_shared<BpTreeNode> (BpTreeNode (attrType ( ), attrLen ( ), pagePtr));

}

inline AttrType IndexHandle::attrType ( ) const {
	return header.attrType;
}

inline size_t IndexHandle::attrLen( )const {
	return header.attrLength;
}

inline size_t IndexHandle::numMaxKeys ( ) const {
	return header.numMaxKeys;
}

inline size_t IndexHandle::numPages ( ) const {
	return header.numPages;
}

inline size_t IndexHandle::height ( ) const {
	return header.height;
}



inline BpTreeNodePtr IndexHandle::FindLeaf (void * pData) {
	if ( root == nullptr )
		return nullptr;

	if ( header.height == 1 ) {
		path[0] = root;
		return root;
	}

	PageNum page;
	RETCODE result;
	PagePtr pagePtr;

	path[0] = root;
	pathPage.push_back (root->GetPageNum ( ));

	for ( size_t i = 1; i < height ( ); i++ ) {
		RecordIdentifier r = path[i - 1]->GetRid (pData);
		size_t pos = path[i - 1]->FindKeyPosFit (pData);

		if ( result = r.GetPageNum (page) ) {
			Utils::PrintRetcode (result);
			return nullptr;
		}
		
		path[i] = FetchNode (page);
		pathPage[i] = pos;

	}

	return path.back();
}
