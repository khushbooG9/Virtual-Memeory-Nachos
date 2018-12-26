/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"
using namespace std;


void Exit_POS(int id)
{   printf("-----------------------------Exit_POS called---------\n");
	kernel->interrupt->SetLevel(IntOff);
	kernel->scheduler->ReadyToRun(kernel->currentThread->parent);
	kernel->currentThread->Finish();
	kernel->interrupt->SetLevel(IntOn);
	cout<<"Finishing the child thread with ID"<<" "<<id<<endl;
}

void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

void ForkTest1(int id)
{
    printf("ForkTest1 is called, its PID is %d\n", id);
    for (int i = 0; i < 3; i++)
    {
        printf("ForkTest1 is in loop %d\n", i);
        for (int j = 0; j < 100; j++)
        {
            // cout<<"Number of times ForkTest1 runnning is "<<j<<endl;
            kernel->interrupt->OneTick();}
    }
    //Exit_POS(id);
}

void ForkTest2(int id)
{
    printf("ForkTest2 is called, its PID is %d\n", id);
    for (int i = 0; i < 3; i++)
    {
        printf("ForkTest2 is in loop %d\n", i);
        for (int j = 0; j < 100; j++){
            // cout<<"Number of times ForkTest2 running is "<<j<<endl;
        kernel->interrupt->OneTick();}
    }
    //Exit_POS(id);
}

void ForkTest3(int id)
{
    printf("ForkTest3 is called, its PID is %d\n", id);
    for (int i = 0; i < 3; i++)
    {
        printf("ForkTest3 is in loop %d\n", i);
        for (int j = 0; j < 100; j++) {
            // cout<<"Number of times FOrkTest3 running is "<<j<<endl;
            kernel->interrupt->OneTick();
        }
    }
    //Exit_POS(id);
}
void SysWrite(int buffer, int size)
{
	int ch;
	

	for(int i= 0; i<size; i++)
	{
        kernel->machine->ReadMem(buffer, 1, &ch);
		printf("%c", * (char *)&ch);
		buffer++;
	}
}

void SysWait_POS(int n)
{
	printf("-----------------------------Wait_POS called---------\n");
	if(kernel->currentThread->childmap.find(n) != kernel->currentThread->childmap.end())
	   {
	   	  cout<<"Invalid child process."<<endl;
	   }
	   else 
	   {
	   	//Thread *child1 = kernel->currentThread->childmap[n];
	   	if(kernel->currentThread->childmap[n]->parentid == -1)
	   	{
	   		cout<<"No parent process is waiting."<<endl;
	   	} else
	   	{
	   		kernel->interrupt->SetLevel(IntOff);
	   		kernel->currentThread->Sleep(FALSE);
	   		//kernel->scheduler->Run(child1, FALSE);
	   		kernel->interrupt->SetLevel(IntOn);

	   		

	   		
	   	}
	   
	   }

}



int SysFork_POS(int n)

	{   printf("-----------------------------Fork_POS called---------\n");
		Thread *child = new Thread("Child forked");
		child->parent = kernel->currentThread;
		int childid = child->getid();
		kernel->currentThread->childmap[childid]= child;
		child->parentid = kernel->currentThread->getid();

		if (n == 1) {
			child->Fork((VoidFunctionPtr)ForkTest1, (void *)childid);
			cout << endl << "ForkTest1 got forked" << endl;
			kernel->currentThread->Yield();
		}
		else if (n == 2) {
			child->Fork((VoidFunctionPtr)ForkTest2, (void *)childid);
			cout << endl << "ForkTest2 got forked" << endl;
			kernel->currentThread->Yield();


		}
		else if (n == 3) {
			child->Fork((VoidFunctionPtr)ForkTest3, (void *)childid);
			cout << endl << "ForkTest3 got forked" << endl;
			kernel->currentThread->Yield();

		}
		else {
			cout << "Invalid entry" << endl;
		}
		return childid;
	}








#endif /* ! __USERPROG_KSYSCALL_H__ */
