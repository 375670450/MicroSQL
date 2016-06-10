#pragma once

/*
	1. �������ڸ�������ɨ��һ��RecordFile�ļ��е�����Records
	2. TODO: ά��һ��PagePtr, ����ÿ�ζ������µ�PagePtr
*/

#include "Utils.hpp"
#include "RecordFile.hpp"

namespace CompMethod {

	bool equal (void * value1, void * value2, AttrType attrtype, int attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast<float*>(value1) == *reinterpret_cast<float*>(value2) );
		case INT: return ( *reinterpret_cast<int*>(value1) == *reinterpret_cast<int*>(value2) );
		default:
			return ( strncmp (reinterpret_cast<char*>(value1), reinterpret_cast<char*>(value2), attrLength) == 0 );
		}
	}

	bool less_than (void * value1, void * value2, AttrType attrtype, int attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast<float*>(value1) < *reinterpret_cast<float*>(value2) );
		case INT: return ( *reinterpret_cast<int*>(value1) < *reinterpret_cast<int*>(value2) );
		default:
			return ( strncmp (reinterpret_cast<char*>(value1), reinterpret_cast<char*>(value2), attrLength) < 0 );
		}
	}

	bool greater_than (void * value1, void * value2, AttrType attrtype, int attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast<float*>(value1) > *reinterpret_cast<float*>(value2) );
		case INT: return ( *reinterpret_cast<int*>(value1) > *reinterpret_cast<int*>(value2) );
		default:
			return ( strncmp (reinterpret_cast<char*>(value1), reinterpret_cast<char*>(value2), attrLength) > 0 );
		}
	}

	bool less_than_or_eq_to (void * value1, void * value2, AttrType attrtype, int attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast<float*>(value1) <= *reinterpret_cast<float*>(value2) );
		case INT: return ( *reinterpret_cast<int*>(value1) <= *reinterpret_cast<int*>(value2) );
		default:
			return ( strncmp (reinterpret_cast<char*>(value1), reinterpret_cast<char*>(value2), attrLength) <= 0 );
		}
	}

	bool greater_than_or_eq_to (void * value1, void * value2, AttrType attrtype, int attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast<float*>(value1) >= *reinterpret_cast<float*>(value2) );
		case INT: return ( *reinterpret_cast<int*>(value1) >= *reinterpret_cast<int*>(value2) );
		default:
			return ( strncmp (reinterpret_cast<char*>(value1), reinterpret_cast<char*>(value2), attrLength) >= 0 );
		}
	}

	bool not_equal (void * value1, void * value2, AttrType attrtype, int attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast<float*>(value1) != *reinterpret_cast<float*>(value2) );
		case INT: return ( *reinterpret_cast<int*>(value1) != *reinterpret_cast<int*>(value2) );
		default:
			return ( strncmp (reinterpret_cast<char*>(value1), reinterpret_cast<char*>(value2), attrLength) != 0 );
		}
	}

}

class FileScan {
public:

	const static PageNum BeginPage = 1;

	enum ScanState {
		Close, Open, End
	};

	struct ScanInfo {

		size_t recordsCount;

		size_t recordsPerPage;

		PageNum scanedPage;

		SlotNum scanedSlot;

		ScanState state;

	};

	FileScan ( );
	~FileScan ( );

	RETCODE OpenScan (const RecordFilePtr &fileHandle,  // Initialize file scan
											  AttrType			attrType,
											  size_t				attrLength,
											  size_t				attrOffset,
											  CompOp        compOp,
											  void          *value);

	RETCODE GetNextRec (Record &rec);                  // Get next matching record

	RETCODE CloseScan ( );                                // Terminate file scan
		
private:
	
	using Comparator = bool (*)( void*, void*, AttrType, int );
	
	using VoidPtr = shared_ptr<void>;

	Comparator _comp;

	RecordFilePtr _recFile;

	PagePtr _curPage;
	
	AttrType _attrType;
	
	size_t _attrLength;

	size_t _attrOffset;

	 VoidPtr _attrValue;

	 ScanInfo _scanInfo;

};

FileScan::FileScan ( ) {
	_scanInfo.state = ScanState::Close;
	_scanInfo.recordsCount = 0;
	_scanInfo.scanedPage = 0;
	_scanInfo.scanedSlot = 0;
	
	_recFile = nullptr;
	_curPage = nullptr;

}

FileScan::~FileScan ( ) {
	
}

inline RETCODE FileScan::OpenScan (const RecordFilePtr & fileHandle, AttrType attrType, size_t attrLength, size_t attrOffset, CompOp compOp, void * value) {
	
	if ( _scanInfo.state == Open )
		return RETCODE::INVALIDSCAN;

	_recFile = fileHandle;

	if ( fileHandle == nullptr || !fileHandle->isValidRecordFile ( ) )
		return RETCODE::INVALIDPAGEFILE;

	RecordFile::RecordFileHeader header;

	_recFile->GetHeader (header);

	if ( value != nullptr ) {			// has condition

		if ( attrType != AttrType::INT && attrType != AttrType::FLOAT && attrType != AttrType::STRING )
			return INVALIDSCAN;

		if ( attrOffset + attrLength > header.recordSize || attrOffset < 0 )
			return RETCODE::INVALIDSCAN;

		switch ( compOp ) {
		case EQ_OP:
			_comp = CompMethod::equal;
			break;
		case LT_OP:
			_comp = CompMethod::less_than;
			break;
		case GT_OP:
			_comp = CompMethod::greater_than;
			break;
		case LE_OP:
			_comp = CompMethod::less_than_or_eq_to;
			break;
		case GE_OP:
			_comp = CompMethod::greater_than_or_eq_to;
			break;
		case NE_OP:
			_comp = CompMethod::not_equal;
			break;
		case NO_OP:
			_comp = nullptr;
			break;
		default:
			return RETCODE::INVALIDSCAN;
			break;
		}

		_attrType = attrType;

		_attrLength = attrLength;

		_attrOffset = attrOffset;

		if ( ( attrType == AttrType::INT || attrType == AttrType::FLOAT ) && attrLength != 4 )
			return RETCODE::INVALIDSCAN;

		_attrValue = shared_ptr<void> (reinterpret_cast< void* >( new char[attrLength] ( ) ));

		memcpy_s (_attrValue.get ( ), attrLength, value, attrLength);

	} 

	// initialize the status
	_scanInfo.state = Open;
	_scanInfo.recordsCount = header.recordSize;
	_scanInfo.scanedPage = BeginPage;
	_scanInfo.scanedSlot = 0;
	
	_curPage = nullptr;

	return RETCODE::COMPLETE;
}

inline RETCODE FileScan::GetNextRec (Record & rec) {

	if ( _scanInfo.state != ScanState::Open )
		return RETCODE::INVALIDSCAN;
	else if ( _scanInfo.state == End )
		return RETCODE::EOFSCAN;
	
	Record tmpRec;
	RETCODE result;
	DataPtr recData;

	for ( ;; ) {

		if ( result = _recFile->GetRec (RecordIdentifier{ _scanInfo.scanedPage, _scanInfo.scanedSlot }, tmpRec) ) {
			Utils::PrintRetcode (result);

			if ( result == RETCODE::EOFFILE ) {
				_scanInfo.state = End;
				return RETCODE::EOFSCAN;
			}
			return result;
		}

		if ( ++_scanInfo.scanedSlot == _scanInfo.recordsPerPage ) {
			_scanInfo.scanedPage++;
			_scanInfo.scanedSlot = 0;
		}

		if ( result = tmpRec.GetData (recData) ) {
			Utils::PrintRetcode (result);
			return result;
		}

		if ( _comp == nullptr || _comp (recData.get ( ), _attrValue.get(), _attrType, _attrLength) ) {	// if satisfies the condition
			rec = tmpRec;
			break;
		}

	}

	return RETCODE::COMPLETE;
}

inline RETCODE FileScan::CloseScan ( ) {
	_scanInfo.state = ScanState::Close;

	return RETCODE::COMPLETE;
}
