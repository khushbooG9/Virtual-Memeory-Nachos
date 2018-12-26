// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "addrspace.h"
#include <map>
#include <list>
using namespace std;

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------
//function to deepcopy the array
void ArrayCopy1(char* source, char* buffer, int epn) {
    for (int i = 0; i < PageSize; i++) {
        buffer[i] = kernel->machine->mainMemory[i + epn * PageSize ];
    }
}

//Function to handle pagefault error
void SysPageFault(int badvadd)
{

	printf("Handling page fault\n");
	int vpn = badvadd/PageSize;
	int freePhyPage = kernel->b->FindAndSet(); //finding the free page in physical memory
	//incase no space in memory
	printf("Free page number: %d \n",freePhyPage );
	if(freePhyPage!= -1){
		
        TranslationEntry *entry = kernel->currentThread->space->Returnpagetableforvpn(vpn);
        
        char *buffer = new char[PageSize];

		kernel->SwapFile->ReadAt(buffer,PageSize,entry->SwapLocation*PageSize);

		bzero(&(kernel->machine->mainMemory[freePhyPage*PageSize]), PageSize);
		memcpy(&(kernel->machine->mainMemory[freePhyPage*PageSize]), buffer, PageSize);
		
		entry->physicalPage = freePhyPage;
		entry->valid = TRUE;
		entry->use = TRUE;
		kernel->PPNtothread[freePhyPage] = kernel->currentThread;
		kernel->PPN->Append(freePhyPage);
		
	}
	else
	{   
		
        int evictpageno = kernel->PPN->RemoveFront();
		printf("size after eviction : %d and evicted pageno is: %d\n", kernel->PPN->NumInList(), evictpageno);
        TranslationEntry *ppnentry = kernel->PPNtothread[evictpageno]->space->Returnpagetableforppn(evictpageno);
        char *buffer = new char[PageSize];

        //copy main memory to buffer
        ArrayCopy1(kernel->machine->mainMemory, buffer, evictpageno);
        
		kernel->SwapFile->WriteAt(buffer, PageSize, ppnentry->SwapLocation*PageSize);

		ppnentry->physicalPage = -1;
		ppnentry->valid = FALSE;
		ppnentry->use = FALSE;
		kernel->b->Clear(evictpageno);
		
	}

}




void
ExceptionHandler(ExceptionType which)
{
    	printf("Inside exception, which: %d\n", which);
	// printf("Inside exception handler\n");
    int type = kernel->machine->ReadRegister(2);
    	printf("Type pf exception is %d\n", type);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    case SyscallException:
      switch(type) {
      case SC_Halt:
	DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

	SysHalt();

	ASSERTNOTREACHED();
	break;

      case SC_Add:
	DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
	
	/* Process SysAdd Systemcall*/
	int result;
	result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			/* int op2 */(int)kernel->machine->ReadRegister(5));

	DEBUG(dbgSys, "Add returning with " << result << "\n");
	/* Prepare Result */
	kernel->machine->WriteRegister(2, (int)result);
	
	/* Modify return point */
	{
	  /* set previous programm counter (debugging only)*/
	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	}

	  case SC_Fork_POS:
	  {
	  	
	  int val = kernel->machine->ReadRegister(4);
	  int childid= SysFork_POS(val);
	  //Thread *child = kernel->currentThread->childmap[childid];
	  kernel->machine->WriteRegister(2, (int)childid); 

	  {
	  /* set previous programm counter (debugging only)*/
	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	}

	return;
	
	ASSERTNOTREACHED();
     } 
	break; 

	case SC_Wait_POS:
	{
		int cid = kernel->machine->ReadRegister(4);
		SysWait_POS(cid);
		{
	  /* set previous programm counter (debugging only)*/
	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	 }
	 return;
	 ASSERTNOTREACHED();
	}
    break;

    case SC_Write:
    {
       int startloc = kernel->machine->ReadRegister(4);
       int size = kernel->machine->ReadRegister(5);
       
       SysWrite(startloc, size);
       {
	  /* set previous programm counter (debugging only)*/
	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	  }
	  return;
	  ASSERTNOTREACHED();
    }
    break;
    case SC_Exit:
    {   
	
		printf("I am ending\n");
		kernel->currentThread->Finish();

    	
   //  	{
	  // /* set previous programm counter (debugging only)*/
	  // kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  // /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  // kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  // /* set next programm counter for brach execution */
	  // kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	  // }
	  return;
    }
    break;

      default:
	  {  cerr << "Unexpected system call " << type << "\n"; }
	
      }
      break;
    case PageFaultException:
    {
    	printf("Inside page fautlt exception, which: %d, type: %d\n", which, type );
    	int badvirad = kernel->machine->ReadRegister(39);
    	SysPageFault(badvirad);
    	return;//
    }
    break;
    
    default:
      { cerr << "Unexpected user mode exception" << (int)which << "\n"; }
      
    }
    ASSERTNOTREACHED();
}
