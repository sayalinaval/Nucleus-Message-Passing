/*** prog1.c ***/
#include "syscall.h"

int
main()
{
	int buff;
	int bufferId = SendMessage("../test/prog2","Hello prog2",-1);
	WaitAnswer(1,"wait ans 1",bufferId);
	Halt();
	Exit(0);
}