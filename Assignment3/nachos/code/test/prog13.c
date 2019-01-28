/*** prog13.c ***/
#include "syscall.h"

int
main()
{
     int buffId = WaitMessage("../test/prog12","wait message 13", -1);
     SendAnswer(1,"Hello prog12",buffId);
     Halt();
     Exit(0);
}
