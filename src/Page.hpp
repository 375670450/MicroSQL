#pragma once

/*
	1. һ��Page��ʵ�ʴ洢��СΪ Utils::PAGESIZE + sizeof(PageHeader)
	2. ��Ȼֻ���ڴ��в�ʹ��Page�ĳ�Ա����, �����ļ��д洢��ʱ��ҲҪ��һ��Page�Ĵ�С��д���ļ�(��Page�洢)
*/


#include "Utils.hpp"
#include <fstream>

class Page {
public:

	struct PageHeader {
		char identifyString[Utils::MAXNAMELEN];
		PageNum pageNum;
		bool isUsed;
	};

	Page ( );
	~Page ( );
	Page (const Page & page);

	RETCODE GetData (DataPtr & pData) const;

	RETCODE GetPageNum (PageNum & pageNum) const;

	bool GetIsUsed ( ) const;

	RETCODE LoadPage (std::ifstream & stream);

	RETCODE SetData (char *);

	RETCODE SetPageNum (PageNum);

	RETCODE SetUsage (bool);

private:

	PageHeader _header;

	DataPtr _pData;							 // points to an address in memory of size Utils::page_size

};

using PagePtr = shared_ptr<Page>;

Page::Page ( ) {
	_header.pageNum = -1;
	_pData = nullptr;
}

Page::~Page ( ) {

}

inline RETCODE Page::GetData (DataPtr & pData) const {
	pData = _pData;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::GetPageNum (PageNum & pageNum) const {
	pageNum = _header.pageNum;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::LoadPage (std::ifstream & stream) {
	RETCODE result;

	stream.read (reinterpret_cast< char* >( &_header ), sizeof (Page::PageHeader));

	if ( strcmp (_header.identifyString, Utils::PAGEIDENTIFYSTRING) != 0 ) {
		return RETCODE::INVALIDPAGE;
	}

	if( _pData == nullptr )
		_pData = shared_ptr<char> (new char[Utils::PAGESIZE] ( ));

	stream.read (_pData.get ( ), sizeof (Utils::PAGESIZE));

	return result;
}


inline bool Page::GetIsUsed ( ) const {
	return _header.isUsed;
}

inline RETCODE Page::SetData (char * pdata) {
	if( _pData == nullptr )
		_pData = shared_ptr<char>( new char[Utils::PAGESIZE]() );			// allocate memory and pointed by a shared pointer
	
	memcpy_s (_pData.get ( ), Utils::PAGESIZE, pdata, Utils::PAGESIZE);			// read the souRETCODEe data
	
	return RETCODE::COMPLETE;
}

inline RETCODE Page::SetPageNum (PageNum num) {
	_header.pageNum = num;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::SetUsage (bool val) {
	_header.isUsed = val;
	return RETCODE::COMPLETE;
}
