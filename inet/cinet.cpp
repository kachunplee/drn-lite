#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#include "cinet.h"

u_long
AtoInetAddr (const char * addr)
{
	struct hostent *host;
	host = gethostbyname(addr);
	if(host)
		return * (u_long *) host->h_addr_list[0];

	return inet_addr(addr);
}
