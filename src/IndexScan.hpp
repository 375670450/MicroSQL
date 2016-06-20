#pragma once

#include "Utils.hpp"
#include "IndexHandle.hpp"
#include "RecordIdentifier.hpp"

class IndexScan {
public:
	IndexScan ( );
	~IndexScan ( );
	
	RETCODE OpenScan (const IndexHandle & indexHandle, // Initialize index scan
										CompOp compOp,
										void * value);
	
	RETCODE GetNextEntry (RecordIdentifier & rid);                         // Get next matching entry
	
	RETCODE CloseScan ( );                                 // Terminate index scan

private:

	//std::vector<>

};

IndexScan::IndexScan ( ) {
}

IndexScan::~IndexScan ( ) {
}
