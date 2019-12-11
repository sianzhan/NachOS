#ifndef VIRTMEM_H
#define VIRTMEM_H

#include <vector>
#include <map>

class VirtualMemoryManager {
	struct EntryKey {
		void *pid;
		unsigned int virtualPage; // Represent the virtual page of pageTable

		// Constructor
		EntryKey(void *_pid, unsigned int _virtualPage) : pid(_pid), virtualPage(_virtualPage) {}
   		bool operator< (const EntryKey& rhs) const;
	};

	private:		
		unsigned int numPages;
		char *virtualMemory;
		bool *isPageUsed; 

		std::map<EntryKey, unsigned int> memoryTable; // Mapping virtual page to page on virtual memory

        // Swap memory from virtual memory to physical memory, both forth and back
	    void Swap(unsigned int virtualPage, unsigned int diskPage, unsigned int physicalPage); 

	public:
	    VirtualMemoryManager(unsigned int numPages);

        // Choose victim page from physical memory
        // Which will be then be used to swap with virtual page
        unsigned int ChooseVictimPage();


		void Fetch(unsigned int virtualPage);

		// Insert a page into virtual memory
		void Put(unsigned int virtualPage, char *data);
};

#endif

