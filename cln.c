#include "global.h"

static char *port = "6666";

static void usage(char *me)
{
	msg("Usage %s <addr> <timeout>", me);
}

static int init_cln_socket(const char *hostname)
{
	int ret, fd = -1, on = 1;
	struct addrinfo hosthints, *hostres, *addrtmp;
	struct protoent *protoent;

	memset(&hosthints, 0, sizeof(struct addrinfo));

	hosthints.ai_family   = AF_UNSPEC;
	hosthints.ai_socktype = SOCK_STREAM;
	hosthints.ai_protocol = IPPROTO_TCP;
	hosthints.ai_flags    = AI_ADDRCONFIG;

	xgetaddrinfo(hostname, port, &hosthints, &hostres);

	for (addrtmp = hostres; addrtmp != NULL ; addrtmp = addrtmp->ai_next) {
		fd = socket(addrtmp->ai_family, addrtmp->ai_socktype, addrtmp->ai_protocol);
		if (fd < 0)
			continue;

		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		ret = (int)connect(fd, addrtmp->ai_addr, addrtmp->ai_addrlen);
		if (ret == -1) {
			msg("Can't connect to %s", hostname);
			exit(EXIT_FAILURE);
		}

		msg("connection established to %s:%s", hostname, port);

		/* great, found a valuable socket */
		break;
	}

	if (fd < 0) {
		msg("Don't found a suitable socket to connect to the client");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(hostres);

	return fd;
}


int main(int ac, char *av[])
{
	int socket, connected_fd, ret;
	struct sockaddr_storage sa;
	socklen_t sa_len = sizeof sa;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	uint32_t len;

	if (ac != 3) {
		usage(av[0]);
		exit(EXIT_FAILURE);
	}

	msg("start client");


	socket = init_cln_socket(av[1]);

}
