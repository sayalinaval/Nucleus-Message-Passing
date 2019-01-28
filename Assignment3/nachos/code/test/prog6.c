/*** prog6.c ***/
#include "syscall.h"

int
main()
{
    int buffId = WaitMessage("../test/prog5","wait message 5", -1);
    buffId = WaitMessage("../test/prog5","wait message 6", buffId);
    SendAnswer(1,"I am right here!",buffId);
    Halt();
    Exit(0);
}
