// alarm.cc
//	Routines to use a hardware timer device to provide a
//	software alarm clock.  For now, we just provide time-slicing.
//
//	Not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "alarm.h"
#include "main.h"

SleepingThread::SleepingThread(Thread *thread, int time)
{
    threadToWake = thread;
    when = time;
}

static int
SleepingCompare (SleepingThread *x, SleepingThread *y)
{
    if (x->when < y->when) { return -1; }
    else if (x->when > y->when) { return 1; }
    else { return 0; }
}


//----------------------------------------------------------------------
// Alarm::Alarm
//      Initialize a software alarm clock.  Start up a timer device
//
//      "doRandom" -- if true, arrange for the hardware interrupts to 
//		occur at random, instead of fixed, intervals.
//----------------------------------------------------------------------

Alarm::Alarm(bool doRandom)
{
    timer = new Timer(doRandom, this);

    sleeping = new SortedList<SleepingThread *>(SleepingCompare);
}

Alarm::~Alarm()
{
    delete timer;
    delete sleeping;
}

void Alarm::WaitUntil(int x)
{
    Interrupt *interrupt = kernel->interrupt;
    MachineStatus oldStatus = interrupt->getStatus();
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    Thread *thread = kernel->currentThread;
    
    DEBUG(dbgThread, "Putting the thread " << thread->getName() << " to sleep for " << x << " ticks");
    ASSERT(time > 0);

    int when = kernel->stats->totalTicks + x;
    SleepingThread *toWake = new SleepingThread(thread, when);

    sleeping->Insert(toWake);

    interrupt->setStatus(SystemMode);
    thread->Sleep(FALSE);
    interrupt->setStatus(oldStatus);

    (void) kernel->interrupt->SetLevel(oldLevel);
}

bool
Alarm::CheckShouldWake()
{
    if (sleeping->IsEmpty()) return FALSE;

    Statistics *stats = kernel->stats;

    SleepingThread *next;
    bool hasWaken = FALSE;

    while (!sleeping->IsEmpty()
          && (sleeping->Front()->when <= stats->totalTicks)) 
    {    
        next = sleeping->RemoveFront();
        kernel->scheduler->ReadyToRun(next->threadToWake);
        hasWaken = TRUE;
    }
    return hasWaken;
}

//----------------------------------------------------------------------
// Alarm::CallBack
//	Software interrupt handler for the timer device. The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	For now, just provide time-slicing.  Only need to time slice 
//      if we're currently running something (in other words, not idle).
//	Also, to keep from looping forever, we check if there's
//	nothing on the ready list, and there are no other pending
//	interrupts.  In this case, we can safely halt.
//----------------------------------------------------------------------

void 
Alarm::CallBack() 
{
    Interrupt *interrupt = kernel->interrupt;
    MachineStatus status = interrupt->getStatus();
    
    bool hasWaken = CheckShouldWake();    
    
    if (status == IdleMode && !hasWaken) {	// is it time to quit?
        if (!interrupt->AnyFutureInterrupts() && sleeping->IsEmpty()) {
	        timer->Disable();	// turn off the timer
	    }
    } else {			// there's someone to preempt
	    interrupt->YieldOnReturn();
    }
}

