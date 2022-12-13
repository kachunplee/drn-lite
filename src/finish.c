#include <unistd.h>

int
main(ac, av)
int ac;
char **av;
{
    setuid(0);
    seteuid(0);
    execv("./finish.pl",av);
	return 1;
}
