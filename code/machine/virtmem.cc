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
    this->swapSpace = new char[numPages * PageSize]();
    this->isPageUsed = new bool[numPages];
    this->frameInfos = new FrameInfo[NumPhysPages];
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
// VirtualMemoryManager::Swap
//      This function swap one virtual page (in swap space) with physical page
//
//      virtualPage : no of virtualPage to be swapped
//      physicalPage: no of physicalPage to be swapped
//----------------------------------------------------------------------

void
VirtualMemoryManager::Swap(unsigned int virtualPage, unsigned int physicalPage) {
    std::map<PagingKey, unsigned int>::iterator itSwapPage = memoryTable.find(PagingKey(kernel->machine->pageTable, virtualPage));

    char *ptrSwapPage = swapSpace + itSwapPage->second * PageSize;
    char *ptrPhysPage = kernel->machine->mainMemory + physicalPage * PageSize;

    char *buffer = new char[PageSize]();

    std::memcpy(buffer, ptrSwapPage, PageSize);
    std::memcpy(ptrSwapPage, ptrPhysPage, PageSize);
    std::memcpy(ptrPhysPage, buffer, PageSize);


    // First, take care of the physical page (which has been moved from main memory to space)

    // Get the frame info of this physical page
    FrameInfo &frameInfo = this->frameInfos[physicalPage];

    // Set this physical page to `invalid` in page table
    frameInfo.pageTable[frameInfo.virtualPage].valid = FALSE;

    // Create record for this physical page, to indicate its new home in swap space
    this->memoryTable[PagingKey(frameInfo.pageTable, frameInfo.virtualPage)] = itSwapPage->second;


    // Second, take care of the virtual page (which has been moved from swap space to main memory)

    // Set the frame info for this virtual page
    frameInfo.pageTable = kernel->machine->pageTable;
    frameInfo.virtualPage = virtualPage;

    // Set this virtual page to `valid` in page table
    TranslationEntry &pageEntry = kernel->machine->pageTable[virtualPage];

    pageEntry.physicalPage = physicalPage; 
    pageEntry.valid = TRUE; // Set to valid
    pageEntry.use = FALSE;
    pageEntry.dirty = FALSE;
    pageEntry.readOnly = FALSE;

    // Delete the record of this virtual page (because it no longer stay in the land of swap space)
    this->memoryTable.erase(itSwapPage);

}


//----------------------------------------------------------------------
// VirtualMemoryManager::Fetch
//      Function to fetch a virtual page from space space and put it into main 
//      memory, make sure the virtual page doesn't exist in main memory
//      before using this function (on PageFaultException), or data loss
//      will occur. (Foolproof check not yet implemented)
//----------------------------------------------------------------------

void
VirtualMemoryManager::Fetch(unsigned int virtualPage) {
    // First try to seek the virtual page from swap space
    PagingKey key = PagingKey(kernel->machine->pageTable, virtualPage);
    std::map<PagingKey, unsigned int>::iterator it = memoryTable.find(key);

    // If the virtual page doesn't exist in swap space nor main memory
    // This would be a write instruction
    // Thus allocate new page for it and then swap a frame out with it, for the instruction
    if (it == memoryTable.end()) {
        this->Put(kernel->machine->pageTable, virtualPage, new char[PageSize]);
    }

    // Swap a physical page into swap space and have the main memory available for this virtual page
    unsigned int physicalPage = this->ChooseVictimPage();

    this->Swap(virtualPage, physicalPage);
}


//----------------------------------------------------------------------
// VirtualMemoryManager::Put
//      Function to find unused swap page
//      then assign it to virtual page
//----------------------------------------------------------------------

void 
VirtualMemoryManager::Put(TranslationEntry *pageTable, unsigned int virtualPage, char *data) {
    unsigned int i = 0;
    while (i < numPages && isPageUsed[i]) ++i;
    if (i == numPages) {
        cerr<<"All swap pages are used"<<endl;
        return;
    }

    isPageUsed[i] = TRUE;
    PagingKey key = PagingKey(pageTable, virtualPage);
    memoryTable[key] = i;
    // Copy the data of size `PageSize` from `data` into swap space.
    std::memcpy(this->swapSpace + i * PageSize, data, PageSize);
    
}


//----------------------------------------------------------------------
// VirtualMemoryManager::opertor<
//      Comparator function for PagingKey
//      Necessary for map to be functional
//----------------------------------------------------------------------

bool
VirtualMemoryManager::PagingKey::operator< (const VirtualMemoryManager::PagingKey& rhs) const
{ 
   return  ((pageTable < rhs.pageTable) || (pageTable == rhs.pageTable && virtualPage < rhs.virtualPage));
}

