/*** prog4.c ***/
#include "syscall.h"

int
main()
{
     int buffId = WaitMessage("../test/prog3","wait message 3", -1);
     SendAnswer(1,"Hello prog3",buffId);
     buffId = WaitMessage("../test/prog3","wait message 4", buffId);
     SendAnswer(1,"I am right here!",buffId);
     Halt();
     Exit(0);
}
