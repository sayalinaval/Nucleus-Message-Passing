/*** prog3.c ***/
#include "syscall.h"

int
main()
{
     int buff;
     int bufferId = SendMessage("../test/prog4","Hello prog4",-1);
     WaitAnswer(1,"wait ans 3",bufferId);
     bufferId = SendMessage("../test/prog4","Where are you",bufferId);
     WaitAnswer(1,"wait ans 4",bufferId);
     Halt();
     Exit(0);
}