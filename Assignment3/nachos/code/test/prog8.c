/*** prog8.c ***/
#include "syscall.h"

int
main()
{
    int buffId = WaitMessage("../test/prog7","wait message 7", -1);
    SendAnswer(1,"I am right here!",buffId);
    Halt();
    Exit(0);
}
