#ifndef VIRTMEM_H
#define VIRTMEM_H

#include <vector>
#include <map>
#include <queue>


// Used to store the informations of frames of main memory
struct FrameInfo {
	TranslationEntry *pageTable;
	unsigned int virtualPage; // Represent the virtual page this frame is mapped from
	unsigned int seq;
	FrameInfo() { seq = -1; }
};

class VirtualMemoryManager {

	enum ReplacementAlgo {
		FIFO,
		LRU
	};
	// This struct store the informations necessary for page replacement algorithm
	struct ReplacementInfo {
		unsigned int physicalPage;
		ReplacementInfo(unsigned int _physicalPage) : physicalPage(_physicalPage) {}
		bool operator< (const ReplacementInfo& rhs) const;
	};	

	// Key used to map virtual page (per process) to their location on swap space
	struct PagingKey {
		TranslationEntry *pageTable;
		unsigned int virtualPage; // Represent the virtual page of pageTable

		// Constructor
		PagingKey(TranslationEntry *_pageTable, unsigned int _virtualPage) : pageTable(_pageTable), virtualPage(_virtualPage) {}
   		bool operator< (const PagingKey& rhs) const;
	};

	private:		
		unsigned int numPages; // The number of page this swap space can hold
		char *swapSpace; // Swap Space
		bool *isPageUsed; // Keep track whether the swap pages have been used.

		std::map<PagingKey, unsigned int> memoryTable; // Map from virtual page to swap page
		std::priority_queue<ReplacementInfo> replacementInfos;
		ReplacementAlgo replacementAlgo;
		unsigned int replacementCounter = 0; // To assign seq to frame, for replacement algo

        // This function swap one virtual page (in swap space) with physical page
	    void Swap(unsigned int virtualPage, unsigned int physicalPage); 

	public:
		FrameInfo* frameInfos; // Keep the informations for every frame of main memory

	    VirtualMemoryManager(unsigned int numPages);

        // Choose victim frame from main memory
        // Which will be then be swapped into swap space, freeing the frame for virtual page
        unsigned int ChooseVictimPage();

		void Fetch(unsigned int virtualPage);

		// Insert a page into swap space
		void Put(TranslationEntry *pageTable, unsigned int virtualPage, char *data);

		// Map a virtual page to physical page
		void Map(TranslationEntry *pageTable, unsigned int virtualPage, unsigned int physicalPage);
};

#endif

