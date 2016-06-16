#pragma once

/*
	1. һ��Page��ʵ�ʴ洢��СΪ Utils::PAGESIZE + sizeof(PageHeader)
	2. ��Ȼֻ���ڴ��в�ʹ��Page�ĳ�Ա����, �����ļ��д洢��ʱ��ҲҪ��һ��Page�Ĵ�С��д���ļ�(��Page�洢)
*/


#include "Utils.hpp"
#include <fstream>

class Page {

	//friend class PageFile;

public:

	struct PageHeader {
		char identifyString[Utils::MAXNAMELEN];
		PageNum pageNum;
		bool isUsed;

		PageHeader(){
			strcpy_s (identifyString, Utils::PAGEIDENTIFYSTRING);
			isUsed = false;
		}

	};

	Page ( );
	~Page ( );
	Page (const Page & page);

	RETCODE GetData (DataPtr & pData) const;

	DataPtr GetData ( ) const;

	RETCODE GetPageNum (PageNum & pageNum) const;

	bool GetIsUsed ( ) const;

	RETCODE OpenPage (std::istream & stream);

	RETCODE SetData (char *);

	RETCODE SetPageNum (PageNum);

	RETCODE SetUsage (bool);

	RETCODE Create (PageNum page = 0);

private:

	PageHeader _header;

	DataPtr _pData;							 // points to an address in memory of size Utils::page_size

};

using PagePtr = shared_ptr<Page>;
using PageNumPtr = shared_ptr<PageNum>;

Page::Page ( ) {
	_header.pageNum = Utils::UNKNOWNPAGENUM;
	_pData = nullptr;
}

Page::~Page ( ) {

}

inline Page::Page (const Page & page) {
	_header = page._header;
	_pData = page._pData;
}

inline RETCODE Page::GetData (DataPtr & pData) const {
	pData = DataPtr(_pData.get ( ) + sizeof (PageHeader));
	return RETCODE::COMPLETE;
}

inline DataPtr Page::GetData ( ) const {
	return DataPtr(_pData.get() + sizeof(PageHeader));
}

inline RETCODE Page::GetPageNum (PageNum & pageNum) const {
	pageNum = _header.pageNum;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::OpenPage (std::istream & stream) {
	//RETCODE result;

	if ( _pData == nullptr )
		this->Create( );

	stream.read (reinterpret_cast< char* >( &_header ), sizeof (Page::PageHeader));

	if ( strcmp (_header.identifyString, Utils::PAGEIDENTIFYSTRING) != 0 ) {
		return RETCODE::INVALIDPAGE;
	}

	stream.read (_pData.get ( ) + sizeof(Page::PageHeader), sizeof (Utils::PAGESIZE));

	return RETCODE::COMPLETE;
}

inline bool Page::GetIsUsed ( ) const {
	return _header.isUsed;
}

inline RETCODE Page::SetData (char * pdata) {
	if( _pData == nullptr )
		_pData = shared_ptr<char>( new char[Utils::PAGESIZE]() );			// allocate memory and pointed by a shared pointer
	
	memcpy_s (_pData.get ( ) + sizeof(PageHeader), Utils::PAGESIZE, pdata, Utils::PAGESIZE);			// read the souRETCODEe data
	
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

inline RETCODE Page::Create (PageNum page) {

	_header.isUsed = true;
	_header.pageNum = page;
	_pData = shared_ptr<char> (new char[Utils::PAGESIZE + sizeof (PageHeader)]);

	return RETCODE::COMPLETE;
}
