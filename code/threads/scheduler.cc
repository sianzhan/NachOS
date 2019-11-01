// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Compare function
//----------------------------------------------------------------------
int PriorityCompare(Thread *a, Thread *b) {
    if(a->getStatus() != NOTARRIVED && b->getStatus() != NOTARRIVED){
        if(a->getPriority() == b->getPriority())
            return 0;
        return a->getPriority() > b->getPriority() ? 1 : -1;
    }

    else{
        return a->getArrivalTime() > b->getArrivalTime() ? 1 : -1;
    }
}

//----------------------------------------------------------------------
// Compare function
//----------------------------------------------------------------------
int BurstTimeCompare(Thread *a, Thread *b) {
    if(a->getStatus() != NOTARRIVED && b->getStatus() != NOTARRIVED){
        if(a->getBurstTime() == b->getBurstTime())
            return 0;
        return a->getBurstTime() > b->getBurstTime() ? 1 : -1;
    }
    else{
        return a->getArrivalTime() > b->getArrivalTime() ? 1 : -1;
    }
    
}

//----------------------------------------------------------------------
// Compare function
//----------------------------------------------------------------------
int DefaultCompare(Thread *a, Thread *b) {
    if(a->getArrivalTime() == b->getArrivalTime() || (a->getStatus() != NOTARRIVED && b->getStatus() != NOTARRIVED))
        return 0;
    return a->getArrivalTime() > b->getArrivalTime() ? 1 : -1;
}

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler()
{
	Scheduler(RR);
}

Scheduler::Scheduler(SchedulerType type)
{
	schedulerType = type;
	switch(schedulerType) {
    	case RR:
        	readyList = new SortedList<Thread *>(DefaultCompare);
        	break;
        case FIFO:
            readyList = new SortedList<Thread *>(DefaultCompare);
            break;
    	case SJF:
    		readyList = new SortedList<Thread *>(BurstTimeCompare);
            break;
        case SRTF:
            readyList = new SortedList<Thread *>(BurstTimeCompare);
            break;
    	case PP:
            readyList = new SortedList<Thread *>(PriorityCompare);
        	break;
        case PNP:
            readyList = new SortedList<Thread *>(PriorityCompare);
            break;
        
   	}
	toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete readyList; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());

    // change status to NOTARRIVED, if arrivalTime is still future
    if(thread->getArrivalTime() > kernel->stats->totalTicks){
        thread->setStatus(NOTARRIVED);
    }
    else{
        thread->setStatus(READY);
    }

    readyList->Insert(thread);
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    // TODO SRTF and PP (checking queue for SRTF and PP)

    if (readyList->IsEmpty()) {
	   return NULL;
    } else {
        // only get thread that wasn't BLOCKED and NOTARRIVED
        ListIterator<Thread* > ptr = ListIterator<Thread*>(readyList);

        while(!ptr.IsDone()){
            if(ptr.Item()->getStatus() == READY)
            {
                readyList->Remove(ptr.Item());
                return ptr.Item();
            }
            ptr.Next();
        }
    	// return readyList->RemoveFront();
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;
 
//	cout << "Current Thread" <<oldThread->getName() << "    Next Thread"<<nextThread->getName()<<endl;
   
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
        toBeDestroyed = oldThread;
    }
    
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	   oldThread->space->SaveState();
    }
#endif
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    if(debug->IsEnabled(dbgKaiZhe)){
        CurrentThreadPrint();
        ReadyListPrint();
    }
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);
    
    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
#ifdef USER_PROGRAM
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	    oldThread->space->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
        toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::ReadyListPrint()
{
    cout << "======Current Ready list contents:======\n";
    readyList->Apply(ThreadPrint);
    // ListIterator<Thread* > ptr = ListIterator<Thread*>(readyList);

    // while(!ptr.IsDone()){
    //     ThreadPrint(ptr.Item());
    //     ptr.Next();
    // }
    cout << "Finish printing list contents.\n\n";
}

void
Scheduler::CurrentThreadPrint(){
    cout << "\n======Current Thread content:======\n";
    ThreadPrint(kernel->currentThread);
    cout << "Finish printing current thread content.\n";
}

void
Scheduler::CheckArrivalTime(){
    readyList->Apply(ThreadCheckArrivalTime);
}


void Scheduler::UpdateBurstTime(){
    kernel->currentThread->UpdateBurstTime();
}

void Scheduler::UpdateWaitingTime(){
    readyList->Apply(ThreadUpdateWaitingTime);
}