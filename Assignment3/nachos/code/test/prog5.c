/*** prog5.c ***/
#include "syscall.h"

int
main()
{
    int buff;
    int bufferId = SendMessage("../test/prog6","Hello prog6",-1);
    bufferId = SendMessage("../test/prog6","Where are you",bufferId);
    WaitAnswer(1,"wait ans 5",bufferId);
    Halt();
    Exit(0);
}