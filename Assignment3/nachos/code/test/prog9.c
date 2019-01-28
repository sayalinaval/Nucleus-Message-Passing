/* prog9.c */
#include "syscall.h"

int
main()
{
     int buff;
     int bufferId = SendMessage("../test/prog10","Hello prog10",-1);
     WaitAnswer(1,"wait ans 9",bufferId);
     buff = SendMessage("../test/prog11","Hello prog11",-1);
     WaitAnswer(1,"wait ans 9",buff);
     Exit(0);
}
