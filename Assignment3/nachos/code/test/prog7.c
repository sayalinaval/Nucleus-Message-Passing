/*** prog7.c ***/
#include "syscall.h"

int
main()
{
	int buff;
	int bufferId = SendMessage("../test/prog8","Hello prog8",-1);
	Exit(0);
}
