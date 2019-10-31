// main.cc 
//	Driver code to initialize, selftest, and run the 
//	operating system kernel.  
//
// Usage: nachos -u -z -d <debugflags> ...
//   -u prints entire set of legal flags
//   -z prints copyright string
//   -d causes certain debugging messages to be printed (cf. debug.h)
//
//  NOTE: Other flags are defined for each assignment, and
//  incorrect flag usage is not caught.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#define MAIN
#include "copyright.h"
#undef MAIN

#include "main.h"
#include <getopt.h>

// global variables
KernelType *kernel;
Debug *debug;


//----------------------------------------------------------------------
// Cleanup
//	Delete kernel data structures; called when user hits "ctl-C".
//----------------------------------------------------------------------

static void 
Cleanup(int x) 
{     
    cerr << "\nCleaning up after signal " << x << "\n";
    delete kernel; 
}


//----------------------------------------------------------------------
// main
// 	Bootstrap the operating system kernel.  
//	
//	Initialize kernel data structures
//	Call selftest procedure
//	Run the kernel
//
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------

int
main(int argc, char **argv)
{
    int i;
    char *debugArg = "";

    // before anything else, initialize the debugging system
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
          printf("===========The following argument is defined in main.cc\n");
          printf("Partial usage: nachos [-z -d debugFlags]\n");
          break;

        case 'z': // print copyright
            printf(copyright);
            break;

        case 'd': // set debug flag
            debugArg = optarg;
            break;

        // done nothing for the flags below, as it might declared in other file (ex : kernel.cc, userkernel.cc)
        case 'r': // set random number
        case 'T': // set scheduler type
        case 'h': // help
        case 'e': // execute file
        case 'p': // set thread's priority
        case 't': // set thread's arrival time
        case 'b': // set thread's burst time
        case 's' : // set debugUserProg to true
          break;

        case ':': // missing option argument
          printf("===========The following argument is defined in main.cc\n");
          printf("%s: option \'-%c\' requires an argument!\n", argv[0], optopt);
          break;

        case '?': // invalid option
        default:
          printf("===========The following argument is defined in main.cc\n");
          printf("%s: option \'-%c\' is invalid: ignored!\n", argv[0], optopt);
          printf("Type ./nachos -h for more help\n");
          break;
      }
    }

    // before anything else, initialize the debugging system
    /*
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
	    ASSERT(i + 1 < argc);   // next argument is debug string
            debugArg = argv[i + 1];
	    i++;
    	} 
        else if (strcmp(argv[i], "-u") == 0) {
                cout << "Partial usage: nachos [-z -d debugFlags]\n";
    	} 
        else if (strcmp(argv[i], "-z") == 0) {
                cout << copyright;
    	}
    }
    */

    debug = new Debug(debugArg);
    
    DEBUG(dbgThread, "Entering main");

    kernel = new KernelType(argc, argv);

    kernel->Initialize();

    CallOnUserAbort(Cleanup);		// if user hits ctl-C

    kernel->SelfTest();
    kernel->Run();


    
    return 0;
}

