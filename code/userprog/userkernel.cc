// userkernel.cc 
//	Initialization and cleanup routines for the version of the
//	Nachos kernel that supports running user programs.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchconsole.h"
#include "userkernel.h"
#include "synchdisk.h"
#include <getopt.h>

//----------------------------------------------------------------------
// UserProgKernel::UserProgKernel
// 	Interpret command line arguments in order to determine flags 
//	for the initialization (see also comments in main.cc)  
//----------------------------------------------------------------------

UserProgKernel::UserProgKernel(int argc, char **argv) 
		: ThreadedKernel(argc, argv)
{
    // init prior, arrivalTime, burstTime
    for(int i=0; i< 10; i++){
	    prior[i] = 0;
	    arrivalTime[i] = 0;
	    burstTime[i] = 1000;
	}

	debugUserProg = FALSE;
	execfileNum=0;

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
        	printf("===========The following argument is defined in userkernel.cc\n");
			printf("Partial usage: nachos [-s]\n");
			printf("Partial usage: nachos [-u]\n");
			printf("Partial usage: nachos [-e] filename\n");
          	break;

        case 'h': // help
			printf("argument 's' is for debugging. Machine status  will be printed\n");
			printf("argument 'e' is for execting file.\n");
			printf("atgument 'u' will print all argument usage.\n");
			printf("For example:\n");
			printf("	./nachos -s : Print machine status during the machine is on.\n");
			printf("	./nachos -e file1 -e file2 : executing file1 and file2.\n");
			break;

		case 's' : // set debugUserProg to true
	    	debugUserProg = TRUE;
	    	break;

        case 'e': // execute file
			execfile[++execfileNum] = optarg;
			break;

        case 'p': // set thread's priority
        	prior[execfileNum] = atoi(optarg);
        	break;

        case 't': // set thread's arrival time
        	arrivalTime[execfileNum] = atof(optarg);
        	break;

        case 'b': // set thread's burst time
        	burstTime[execfileNum] = atoi(optarg);
        	break;

        // done nothing for the flags below, as it might declared in other file (ex : main.cc, kernel.cc)
        case 'z': // print copyright
        case 'd': // set debug flag
        case 'r': // set random number
        case 'T': // set scheduler type
         	break;

        case ':': // missing option argument
          printf("===========The following argument is defined in userkernel.cc\n");
          printf("%s: option \'-%c\' requires an argument!\n", argv[0], optopt);
          break;

        case '?': // invalid option
        default:
          printf("===========The following argument is defined in userkernel.cc\n");
          printf("%s: option \'-%c\' is invalid: ignored!\n", argv[0], optopt);
          printf("Type ./nachos -h for more help\n");
          break;
      }
    }

	/*
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
	    debugUserProg = TRUE;
	}
	else if (strcmp(argv[i], "-e") == 0) {
		execfile[++execfileNum]= argv[++i];
	}
    	 else if (strcmp(argv[i], "-u") == 0) {
		cout << "===========The following argument is defined in userkernel.cc" << endl;
		cout << "Partial usage: nachos [-s]\n";
		cout << "Partial usage: nachos [-u]" << endl;
		cout << "Partial usage: nachos [-e] filename" << endl;
	}
	else if (strcmp(argv[i], "-h") == 0) {
		cout << "argument 's' is for debugging. Machine status  will be printed " << endl;
		cout << "argument 'e' is for execting file." << endl;
		cout << "atgument 'u' will print all argument usage." << endl;
		cout << "For example:" << endl;
		cout << "	./nachos -s : Print machine status during the machine is on." << endl;
		cout << "	./nachos -e file1 -e file2 : executing file1 and file2."  << endl;
	}
    }
    */
}

//----------------------------------------------------------------------
// UserProgKernel::Initialize
// 	Initialize Nachos global data structures.
//----------------------------------------------------------------------

void
UserProgKernel::Initialize()
{
    ThreadedKernel::Initialize();	// init multithreading

    machine = new Machine(debugUserProg);
    fileSystem = new FileSystem();
#ifdef FILESYS
    synchDisk = new SynchDisk("New SynchDisk");
#endif // FILESYS
}

//----------------------------------------------------------------------
// UserProgKernel::~UserProgKernel
// 	Nachos is halting.  De-allocate global data structures.
//	Automatically calls destructor on base class.
//----------------------------------------------------------------------

UserProgKernel::~UserProgKernel()
{
    delete fileSystem;
    delete machine;
#ifdef FILESYS
    delete synchDisk;
#endif
}

//----------------------------------------------------------------------
// UserProgKernel::Run
// 	Run the Nachos kernel.  For now, just run the "halt" program. 
//----------------------------------------------------------------------
void
ForkExecute(Thread *t)
{
	t->space->Execute(t->getName());
}

void
UserProgKernel::Run()
{
	/*
	cout << "\n========Prior value\n";
	for(int i=0; i< 10; i++){
		cout << prior[i] << '\t';
	}
	cout << "\n========\n";
	cout << "\n========Burst time value\n";
	for(int i=0; i< 10; i++){
		cout << burstTime[i] << '\t';
	}
	cout << "\n========\n";
	*/

	cout << "Total threads number is " << execfileNum << endl;
	for (int n=1;n<=execfileNum;n++)
	{
		t[n] = new Thread(execfile[n]);
		t[n]->space = new AddrSpace();
		t[n]->setPriority(prior[n]);
		t[n]->setBurstTime(burstTime[n]);
		t[n]->setArrivalTime(arrivalTime[n] == 0 ? 0 : arrivalTime[n] * 10 + 50);
		t[n]->Fork((VoidFunctionPtr) &ForkExecute, (void *)t[n]);

		if(t[n]->getArrivalTime() > 0){
			t[n]->setStatus(NOTARRIVED);
			// cout << "Thread " << execfile[n] << " is not arrived yet! Arriving Time is " << t[n]->getArrivalTime() << endl;
			cout << "Thread " << execfile[n] << " is not arrived yet!" << endl;
		}
		else{
			cout << "Thread " << execfile[n] << " is executing." << endl;
		}

		if(debug->IsEnabled(dbgKaiZhe)){
			kernel->scheduler->CurrentThreadPrint();
		    kernel->scheduler->ReadyListPrint();
		}
	}
	if(debug->IsEnabled(dbgKaiZhe)){
		kernel->scheduler->CurrentThreadPrint();
	    kernel->scheduler->ReadyListPrint();
	}

//	Thread *t1 = new Thread(execfile[1]);
//	Thread *t1 = new Thread("../test/test1");
//	Thread *t2 = new Thread("../test/test2");

//    AddrSpace *halt = new AddrSpace();
//	t1->space = new AddrSpace();
//	t2->space = new AddrSpace();

//    halt->Execute("../test/halt");
//	t1->Fork((VoidFunctionPtr) &ForkExecute, (void *)t1);
//	t2->Fork((VoidFunctionPtr) &ForkExecute, (void *)t2);
    ThreadedKernel::Run();
//	cout << "after ThreadedKernel:Run();" << endl;	// unreachable
}

//----------------------------------------------------------------------
// UserProgKernel::SelfTest
//      Test whether this module is working.
//----------------------------------------------------------------------

void
UserProgKernel::SelfTest() {
/*    char ch;

    ThreadedKernel::SelfTest();

    // test out the console device

    cout << "Testing the console device.\n" 
	<< "Typed characters will be echoed, until q is typed.\n"
    	<< "Note newlines are needed to flush input through UNIX.\n";
    cout.flush();

    SynchConsoleInput *input = new SynchConsoleInput(NULL);
    SynchConsoleOutput *output = new SynchConsoleOutput(NULL);

    do {
    	ch = input->GetChar();
    	output->PutChar(ch);   // echo it!
    } while (ch != 'q');

    cout << "\n";

    // self test for running user programs is to run the halt program above
*/




//	cout << "This is self test message from UserProgKernel\n" ;
}


// print statistics of the process waiting time and burst time
void
UserProgKernel::Print(){
	cout << "\n----------------\nBonus Part!\n";
	int totalWaitingTime = 0;
	int totalTurnAroundTime = 0;
	for (int n=1;n<=execfileNum;n++)
	{
		if(t[n]){
			cout <<  "ThreadName: " << t[n]->getName() << '\t'; 
		    cout <<  "Used Burst Time: " << t[n]->getRealBurstTime() << '\t'; 
		    cout << "Waiting Time: " << t[n]->getWaitingTime() << '\t';
		    cout << "Turn around Time: " << t[n]->getRealBurstTime() + t[n]->getWaitingTime() << '\n';
		    totalTurnAroundTime += t[n]->getRealBurstTime() + t[n]->getWaitingTime();
		    totalWaitingTime += t[n]->getWaitingTime();
		}
	}
	cout << "Average Waiting Time: " << (double)totalWaitingTime / execfileNum << '\n';
	cout << "Average Turn Around Time: " << (double)totalTurnAroundTime / execfileNum << '\n';
	cout << "----------------\n";
}
