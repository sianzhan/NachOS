#ifndef VIRTMEM_H
#define VIRTMEM_H

#include <vector>
#include <map>

// Used to store the informations of frame on main memory
struct FrameInfo{
	TranslationEntry *pageTable;
	unsigned int virtualPage; // Represent the virtual page this frame is mapped from
};

class VirtualMemoryManager {
	struct EntryKey {
		TranslationEntry *pageTable;
		unsigned int virtualPage; // Represent the virtual page of pageTable

		// Constructor
		EntryKey(TranslationEntry *_pageTable, unsigned int _virtualPage) : pageTable(_pageTable), virtualPage(_virtualPage) {}
   		bool operator< (const EntryKey& rhs) const;
	};

	private:		
		unsigned int numPages; // The number of page this virtual memory has
		char *virtualMemory; // The Virtual Memory
		bool *isPageUsed; // Keep track whether the pages of virtual memory have been used.

		std::map<EntryKey, unsigned int> memoryTable; // Map of virtual page to page of virtual memory

        // Swap memory from virtual memory to physical memory, both forth and back
	    void Swap(unsigned int virtualPage, unsigned int diskPage, unsigned int physicalPage); 

	public:
		FrameInfo* frameInfos; // Keep the FrameInfo for every frame on main memory

	    VirtualMemoryManager(unsigned int numPages);

        // Choose victim page from physical memory
        // Which will be then be used to swap with virtual page
        unsigned int ChooseVictimPage();

		void Fetch(unsigned int virtualPage);

		// Insert a page into virtual memory
		void Put(TranslationEntry *pageTable, unsigned int virtualPage, char *data);
};

#endif

