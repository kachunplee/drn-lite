#include <sys/types.h>
#include <unistd.h>

void main (ac, av)
int ac;
char **av;
{
    setuid(0);
    seteuid(0);
    execv("/var/local/www/bin/cleancache.sh", av);
}
