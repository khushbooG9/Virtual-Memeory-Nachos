#include "kernel.h"
#include "main.h"
#include "thread.h"

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        //cout<<kernel->currentThread->getThreadID()<<endl;
        kernel->currentThread->Yield();
    }
}

void
ThreadTest()
{
    Thread *t = new Thread("forked thread");
    t->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
    //cout<<t->getThreadID()<<endl;
    
    SimpleThread(0);
    Thread *t1 = new Thread("forked thread");
    t1->Fork((VoidFunctionPtr) SimpleThread, (void *) 900);
    //cout<<t1->getThreadID()<<endl;
    Thread *t2 = new Thread("forked thread");
    t2->Fork((VoidFunctionPtr) SimpleThread, (void *) 10000);
    //cout<<t2->getThreadID()<<endl;

}
