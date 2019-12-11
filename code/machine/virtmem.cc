#include "copyright.h"
#include "machine.h"
#include "main.h"
#include "virtmem.h"
#include <cstring>
#include <stdlib.h>

//----------------------------------------------------------------------
// VirtualMemoryManager::VirtualMemoryManager
//      Constructor
//----------------------------------------------------------------------

VirtualMemoryManager::VirtualMemoryManager(unsigned int numPages) {
    this->numPages = numPages;
    this->virtualMemory = new char[numPages * PageSize]();
    this->isPageUsed = new bool[numPages];
}


//----------------------------------------------------------------------
// VirtualMemoryManager::ChooseVictimPage
//      #TODO
//      Function which determine which victim page on physical memory to be
//      swapped with.
//----------------------------------------------------------------------

unsigned int 
VirtualMemoryManager::ChooseVictimPage() {
    return rand() % NumPhysPages; // Select randomly currently.
}


//----------------------------------------------------------------------
// VirtualMemoryManager::Fetch
//      This function fetches virtual pages that are invalid, from 
//      virtual memory onto main memory
//
//      virtualPage : virtualPage to be swapped onto main memory
//      diskPage    : where the virtual page is currently located on virtual memory,
//                    also where the original physical page will be swapped to
//      physicalPage: where the virtualPage will be swapped on main memory, 
//                    and the original page will be swapped into virtual memory
//----------------------------------------------------------------------

void
VirtualMemoryManager::Swap(unsigned int virtualPage, unsigned int diskPage, unsigned int physicalPage) {
    char *ptrDiskPage = this->virtualMemory + diskPage * PageSize;
    char *ptrPhysPage = kernel->machine->mainMemory + physicalPage * PageSize;

    char *buffer = new char[PageSize]();

    std::memcpy(buffer, ptrDiskPage, PageSize);
    std::memcpy(ptrDiskPage, ptrPhysPage, PageSize);
    std::memcpy(ptrPhysPage, buffer, PageSize);

    // First set the table entry of the original page to INVALID
    unsigned int i;
    for (i = 0; i < kernel->machine->pageTableSize; ++i) {
        if (kernel->machine->pageTable[i].physicalPage == physicalPage) {
            kernel->machine->pageTable[i].physicalPage = -1;
            kernel->machine->pageTable[i].valid = FALSE; // Mark this page as invaid
            kernel->machine->pageTable[i].use = FALSE;
            kernel->machine->pageTable[i].dirty = FALSE;
            kernel->machine->pageTable[i].readOnly = FALSE;
            break;
        }
    }
    if (i == kernel->machine->pageTableSize) {
        cout<<"BOOOOOOOOOOOOOOO"<<endl;
        // Exception
    } else {
        this->memoryTable[EntryKey(kernel->currentThread, i)] = diskPage;
    }

    // Then set the table entry of new virtual page
    TranslationEntry &pageEntry = kernel->machine->pageTable[virtualPage];

    pageEntry.physicalPage = physicalPage;
    pageEntry.valid = TRUE;
    pageEntry.use = FALSE;
    pageEntry.dirty = FALSE;
    pageEntry.readOnly = FALSE;

    // Remove the virtual memory record of the page swappped into main memory
    std::map<EntryKey, unsigned int>::iterator it = memoryTable.find(EntryKey(kernel->currentThread, virtualPage));
    this->memoryTable.erase(it);

}


//----------------------------------------------------------------------
// VirtualMemoryManager::Fetch
//      Function to fetch virtual page from virtual memory(disk) into main 
//      memory, make sure the virtual page doesn't exist in main memory 
//      before using this function (on PageFaultException), or data loss
//      will occur. (Foolproof check not yet implemented)
//----------------------------------------------------------------------

void
VirtualMemoryManager::Fetch(unsigned int virtualPage) {
    // First get the page in virtual memory for the virtual page
    EntryKey key = EntryKey(kernel->currentThread, virtualPage);
    std::map<EntryKey, unsigned int>::iterator it = memoryTable.find(key);

    // If the virtualPage address doesn't exist in virtualMemory nor mainMemory
    // This would be a Write instruction
    // Thus allocate new page for it and then swap a frame out for the instruction
    if (it == memoryTable.end()) {
        this->Put(virtualPage, new char[PageSize]);
    }

    // Swap a frame out into virtual memory and have the main memory available for this virtual address
    unsigned int diskPage = memoryTable.find(key)->second;
    unsigned int physicalPage = this->ChooseVictimPage();

    this->Swap(virtualPage, diskPage, physicalPage);
}


//----------------------------------------------------------------------
// VirtualMemoryManager::Put
//      Function which find unused page on virtual memory
//      then allocate it to virtual page
//----------------------------------------------------------------------

void 
VirtualMemoryManager::Put(unsigned int virtualPage, char *data) {
    unsigned int i = 0;
    while (i < numPages && isPageUsed[i]) ++i;
    if (i == numPages) {
        cerr<<"All Virtual memory space used"<<endl;
        return;
    }

    isPageUsed[i] = TRUE;
    EntryKey key = EntryKey(kernel->currentThread, virtualPage);
    memoryTable[key] = i;
    // Copy the data of size `PageSize` from `data` into virtual memory.
    std::memcpy(this->virtualMemory + i * PageSize, data, PageSize);
    
}


//----------------------------------------------------------------------
// VirtualMemoryManager::opertor<
//      Comparator function for EntryKey
//      Necessary for map to be functional
//----------------------------------------------------------------------

bool
VirtualMemoryManager::EntryKey::operator< (const VirtualMemoryManager::EntryKey& rhs) const
{ 
   return  ((pid < rhs.pid) || (pid == rhs.pid && virtualPage < rhs.virtualPage));
}

