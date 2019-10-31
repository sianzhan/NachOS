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
#include <ctime>

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
    bool hasReadyThread = _threadCollection.HasReadyThread();
    
    kernel->currentThread->setPriority(kernel->currentThread->getPriority() - 1);
    if (status == IdleMode && !hasReadyThread && _threadCollection.IsEmpty()) 
    {	// is it time to quit?
        if (!interrupt->AnyFutureInterrupts()) {
	    timer->Disable();	// turn off the timer
	}
    } else 
    {			// there's someone to preempt
    	if(kernel->scheduler->getSchedulerType() == RR ||
                kernel->scheduler->getSchedulerType() == PP ) {
            interrupt->YieldOnReturn();
        }
    }
}

void Alarm::WaitUntil(int x)
{
    // save previous level
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);

    Thread *thread = kernel->currentThread;
    std::cout << "Alarm::WaitUntil Called! Thread go to sleep" << endl;
    _threadCollection.PutThreadSleep(thread, x);
    // set back to previous level
    kernel->interrupt->SetLevel(oldLevel);
}

void ThreadInSleepCollection::PutThreadSleep(Thread* _thread, int duration)
{
    // check the current interrupt level
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    _threads.push_back(ThreadInSleep(_thread, duration + _currentTime));
    // set it to sleep, but not finishing (so passing false to the first parameter)
    _thread->Sleep(false);
}

bool ThreadInSleepCollection::HasReadyThread()
{
    bool rtn = false;
    _currentTime++;

    for(std::list<ThreadInSleep>::iterator it = _threads.begin(); it != _threads.end();)
    {
        if(_currentTime >= it->timeToWake)
        {
            rtn = true;
            std::cout << "Has Ready Thread! Thread woke up!" << endl;
            kernel->scheduler->ReadyToRun(it->thread);
            it = _threads.erase(it);
        }
        else
        {
            it++;
        }
    }

    return rtn;
}

bool ThreadInSleepCollection::IsEmpty()
{
    return _threads.size() == 0;
}

