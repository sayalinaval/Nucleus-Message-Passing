/* prog11.c */
#include "syscall.h"

int
main()
{
	int buffId = WaitMessage("../test/prog9","wait message 9", -1);
	SendAnswer(1,"Hello prog9 from prog11",buffId);
	Exit(0);
}