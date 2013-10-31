#include "global.h"

static char *port = "6666";

static void usage(void)
{
	fprintf(stderr, "srv");
}

static int init_srv_socket(void)
{
	int ret, fd = -1, on = 1;
	struct addrinfo hosthints, *hostres, *addrtmp;
	struct protoent *protoent;

	memset(&hosthints, 0, sizeof(struct addrinfo));

	hosthints.ai_family   = AF_UNSPEC;
	hosthints.ai_socktype = SOCK_STREAM;
	hosthints.ai_protocol = IPPROTO_TCP;
	hosthints.ai_flags    = AI_ADDRCONFIG | AI_PASSIVE;

	xgetaddrinfo(NULL, port, &hosthints, &hostres);

	for (addrtmp = hostres; addrtmp != NULL ; addrtmp = addrtmp->ai_next) {
		fd = socket(addrtmp->ai_family, addrtmp->ai_socktype, addrtmp->ai_protocol);
		if (fd < 0)
			continue;

		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		ret = bind(fd, addrtmp->ai_addr, (int)addrtmp->ai_addrlen);
		if (ret) {
			msg("bind failed");
                        close(fd);
                        fd = -1;
			continue;
		}

		ret = listen(fd, 1);
		if (ret < 0) {
			msg("bind failed");
			close(fd);
			fd = -1;
			continue;
		}

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

	msg("start server");

	socket = init_srv_socket();

	msg("block in accept");
	connected_fd = accept(socket, (struct sockaddr *) &sa, &sa_len);
	if (connected_fd == -1) {
		err("accept error: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	ret = getnameinfo((struct sockaddr *)&sa, sa_len, hbuf,
			NI_MAXHOST, sbuf, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	if (ret != 0) {
		msg("getnameinfo error: %s",  gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	msg("connection established from %s:%s", hbuf, sbuf);
}
