// kernel.cc 
//	Initialization and cleanup routines for the Nachos kernel.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "main.h"
#include "kernel.h"
#include "sysdep.h"
#include "synch.h"
#include "synchlist.h"
#include "libtest.h"
#include "elevatortest.h"
#include "string.h"
#include <getopt.h>

//----------------------------------------------------------------------
// ThreadedKernel::ThreadedKernel
// 	Interpret command line arguments in order to determine flags 
//	for the initialization (see also comments in main.cc)  
//----------------------------------------------------------------------

ThreadedKernel::ThreadedKernel(int argc, char **argv)
{
    randomSlice = FALSE; 
    //default as RR
    type = RR;

// remember to refresh the scanning position to front
    optind = 0;
    while(1){
      // for tutorial about getopt:
      // http://www.informit.com/articles/article.aspx?p=175771&seqNum=3
      int option_index = 0;
      static struct option long_options[] = {
          {"help", no_argument, 0 , 'h'},
          {"usage", no_argument, 0, 'u'},
          {"debUsr", no_argument, 0, 's'},
          {"copyright", no_argument, 0, 'z'},

          {"debug", required_argument, 0, 'd'},
          {"exec", required_argument, 0, 'e'},
          {"prior", required_argument, 0, 'p'},
          {"arrivalTime", required_argument, 0, 't'},
          {"burstTime", required_argument, 0, 'b'},
          {"type", required_argument, 0, 'T'},
          {"rs", required_argument, 0 , 'r'},
          {0, 0, 0, 0},
      };

      char c = getopt_long(argc, argv, ":huszd:e:p:t:b:T:r:", long_options, &option_index);
      // end position reached
      if(c == -1){
        // remember to refresh the scanning position to front
        optind = 0;
        break;
      }

      switch(c){
        case 'u': // prints entire set of legal flags
          printf("===========The following argument is defined in kernel.cc\n");
          printf("Partial usage: nachos [-rs randomSeed]\n");
          break;

        case 'r':
          RandomInit(atoi(optarg));// initialize pseudo-random
          // number generator
          randomSlice = TRUE;
          printf("Number is : %s\n", optarg);
          break;

        case 'T': // set scheduler type
          if(strcmp(optarg, "RR") == 0) {
            type = RR;
          } 
          else if (strcmp(optarg, "FCFS") == 0) {
            type = FIFO;
          } 
          else if (strcmp(optarg, "PNP") == 0) {
            type = PNP;
          }
          else if (strcmp(optarg, "PP") == 0) {
            type = PP;
          } 
          else if (strcmp(optarg, "SJF") == 0) {
            type = SJF;
          }
          else if (strcmp(optarg, "SRTF") == 0) {
            type = SRTF;
          } 
          else{
            printf("Invalid Scheduler type: %s\n", optarg);
          }
          break;

        // done nothing for the flags below, as it might declared in other file (ex : main.cc, userkernel.cc)
        case 'h': // help
        case 'e': // execute file
        case 'p': // set thread's priority
        case 't': // set thread's arrival time
        case 'b': // set thread's burst time
        case 's' : // set debugUserProg to true
        case 'd' : // set debug flag
        case 'z': // copyright
          break;

        case ':': // missing option argument
          printf("===========The following argument is defined in kernel.cc\n");
          printf("%s: option \'-%c\' requires an argument!\n", argv[0], optopt);
          break;

        case '?': // invalid option
        default:
          printf("===========The following argument is defined in kernel.cc\n");
          printf("%s: option \'-%c\' is invalid: ignored!\n", argv[0], optopt);
          printf("Type ./nachos -h for more help\n");
          break;
      }
    }

/*
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-rs") == 0) {
        ASSERT(i + 1 < argc);
        RandomInit(atoi(argv[i + 1]));// initialize pseudo-random
        // number generator
        randomSlice = TRUE;
        i++;
      } 
      else if (strcmp(argv[i], "-u") == 0) {
        cout << "Partial usage: nachos [-rs randomSeed]\n";
      } 
      else if(strcmp(argv[i], "RR") == 0) {
        type = RR;
      } 
      else if (strcmp(argv[i], "FCFS") == 0) {
        type = FIFO;
      } 
      else if (strcmp(argv[i], "PRIORITY") == 0) {
        type = Priority;
      } 
      else if (strcmp(argv[i], "SJF") == 0) {
        type = SJF;
      }
    }
*/

}

//----------------------------------------------------------------------
// ThreadedKernel::Initialize
// 	Initialize Nachos global data structures.  Separate from the 
//	constructor because some of these refer to earlier initialized
//	data via the "kernel" global variable.
//----------------------------------------------------------------------

void
ThreadedKernel::Initialize()
{
    stats = new Statistics();		// collect statistics
    interrupt = new Interrupt;		// start up interrupt handling
    scheduler = new Scheduler(type);	// initialize the ready queue
    alarm = new Alarm(randomSlice);	// start up time slicing

    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread("main");		
    currentThread->setStatus(RUNNING);

    interrupt->Enable();
}

//----------------------------------------------------------------------
// ThreadedKernel::~ThreadedKernel
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------

ThreadedKernel::~ThreadedKernel()
{
    delete alarm;
    delete scheduler;
    delete interrupt;
    delete stats;
    
    Exit(0);
}

//----------------------------------------------------------------------
// ThreadedKernel::Run
// 	Run the Nachos kernel.  For now, do nothing. In practice,
//	after initializing data structures, the kernel would
//	start a user program to print the login prompt.
//----------------------------------------------------------------------

void
ThreadedKernel::Run()
{
    // NOTE: if the procedure "main" returns, then the program "nachos"
    // will exit (as any other normal program would).  But there may be
    // other threads on the ready list (started in SelfTest).  
    // We switch to those threads by saying that the "main" thread 
    // is finished, preventing it from returning.
    currentThread->Finish();	
    // not reached
}

//----------------------------------------------------------------------
// ThreadedKernel::SelfTest
//      Test whether this module is working.
//----------------------------------------------------------------------

void
ThreadedKernel::SelfTest() {
   Semaphore *semaphore;
   SynchList<int> *synchList;
   
   LibSelfTest();		// test library routines
   
   currentThread->SelfTest();	// test thread switching
   
   				// test semaphore operation
   semaphore = new Semaphore("test", 0);
   semaphore->SelfTest();
   delete semaphore;
   
   				// test locks, condition variables
				// using synchronized lists
   synchList = new SynchList<int>;
   synchList->SelfTest(9);
   delete synchList;

   ElevatorSelfTest();
}
