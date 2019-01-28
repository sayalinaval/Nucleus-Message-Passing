/*** prog2.c ***/
#include "syscall.h"

int
main()
{
	int buffId = WaitMessage("../test/prog1","wait message 1", -1);
	SendAnswer(1,"Hello prog1",buffId);
	Halt();
	Exit(0);
}
