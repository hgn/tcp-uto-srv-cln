#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <float.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <inttypes.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

static double xgettimeofday(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

static double ref_time = DBL_MAX;


static void msg(const char *format, ...)

{
	va_list ap;
	double time;

	time = xgettimeofday();

	if (ref_time == DBL_MAX)
		ref_time = time;

	fprintf(stdout, "[%.5lf] ", time - ref_time);

	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);

	fputs("\n", stdout);
        fflush(stdout);
}


static void xgetaddrinfo(const char *node, const char *service,
		struct addrinfo *hints, struct addrinfo **res)
{
	int ret;

	ret = getaddrinfo(node, service, hints, res);
	if (ret != 0) {
		msg("call to getaddrinfo() failed: %s!",
		    (ret == EAI_SYSTEM) ?  strerror(errno) : gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	return;
}


static ssize_t write_len(int fd, const void *buf, size_t len)
{
	const char *bufptr = buf;
	ssize_t total = 0;

	if (len == 0)
		return 0;

	do {
		ssize_t written = write(fd, bufptr, len);
		if (written < 0) {
			int real_errno;

			if (errno == EINTR || errno == EAGAIN)
				continue;

			real_errno = errno;
			msg("Could not write %u bytes: %s", len, strerror(errno));
			errno = real_errno;
			break;
		}
		total  += written;
		bufptr += written;
		len    -= written;
	} while (len > 0);

	return total > 0 ? 0 : -1;
}


static ssize_t read_len(int fd, const void *buf, size_t len)
{
	const char *bufptr = buf;
	size_t read_actual = 0;

	if (len == 0)
		return 0;

	while (1) {

		ssize_t cur = read(fd, (void *)bufptr, len - read_actual);

		if (cur < 0) {
			int real_errno;

			if (errno == EINTR || errno == EAGAIN)
				continue;

			real_errno = errno;
			msg("Could not read %u bytes: %s", len - read_actual, strerror(errno));
			errno = real_errno;
			break;
		}

		if (cur == 0) {
			msg("read return 0, read_actual: %u", read_actual);
			return 0;
		}

		bufptr += cur; read_actual += cur;

		if (read_actual >= len)
			break;
	}

	return read_actual;
}
