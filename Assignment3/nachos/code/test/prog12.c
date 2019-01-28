/*** prog12.c ***/
#include "syscall.h"

int
main()
{
     int buff;
     int bufferId = SendMessage("../test/prog13","Hello prog13",-1);
     WaitAnswer(1,"wait ans 12",bufferId);
     bufferId = SendMessage("../test/prog13","Where are you",bufferId);
     WaitAnswer(1,"wait ans 13",bufferId);
     Halt();
     Exit(0);
}