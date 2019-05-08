#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "dbg.h"
#include <errno.h>

void perr_exit(const char *s)
{
	perror(s);
	exit(-1);
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int n;

again:
	if ((n = accept(fd, sa, salenptr)) < 0) {
		if ((errno == ECONNABORTED) || (errno == EINTR))
			goto again;
		else
			log_err("accept error");
	}
	return n;
}

int Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
    int n;

	if ((n = bind(fd, sa, salen)) < 0)
		log_err("bind error");

    return n;
}

int Connect(int fd, struct sockaddr *sa, socklen_t salen)
{
	int     flags, n, error;
    socklen_t len;
    fd_set rset, wset;
    struct timeval tval;
	int nsec=3;
    error = 0;
    if ( (n = connect(fd, sa, salen)) < 0)
        if (errno != EINPROGRESS)
            return (-1);
 
    /* Do whatever we want while the connect is taking place. */
 
    if (n == 0)
        goto done;               /* connect completed immediately */
 
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    wset = rset;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;
 
    if ( (n = select(fd + 1, &rset, &wset, NULL,
                    nsec ? &tval : NULL)) == 0) {
        close(fd);          /* timeout */
        errno = ETIMEDOUT;
        return (-1);
    }
 
    if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset)) {
        len = sizeof(error);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            return (-1);     /* Solaris pending error */
    } else
	{
		log_err("select error: sockfd not set");
	}
 
  done:
   // Fcntl(fd, F_SETFL, flags);  /* restore file status flags */
 
    if (error) {
        close(fd);           /* just in case */
        errno = error;
        return (-1);
    }
    return (0);
}	
int Listen(int fd, int backlog)
{
    int n;

	if ((n = listen(fd, backlog)) < 0)
		log_err("listen error");

    return n;
}

int Socket(int family, int type, int protocol)
{
	int n;

	if ((n = socket(family, type, protocol)) < 0)
		log_err("socket error");

	return n;
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t n;
again:
	if ( (n = read(fd, ptr, nbytes)) == -1) {
		if (errno == EINTR ||errno == EAGAIN || errno == EWOULDBLOCK)
			{
				goto again;
				usleep(100000);
			}
		else
			return -1;
	}
	return n;
}

ssize_t Write(int fd, const void *ptr, size_t nbytes)
{
	ssize_t n;

again:
	if ( (n = write(fd, ptr, nbytes)) == -1) {
		if (errno == EINTR)
			goto again;
		else
			return -1;
	}
	return n;
}

int Close(int fd)
{
    int n;
	if ((n = close(fd)) == -1)
		log_err("close error");

    return n;
}

/*参三: 应该读取的字节数*/
ssize_t Readn(int fd, void *vptr, size_t n)
{
	size_t  nleft;              //usigned int 剩余未读取的字节数
	ssize_t nread;              //int 实际读到的字节数
	char   *ptr;

	ptr = vptr;
	nleft = n;

	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		} else if (nread == 0)
			break;

		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;
}

ssize_t Writen(int fd, const void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}

		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}

static ssize_t my_read(int fd, char *ptr)
{
	static int read_cnt;
	static char *read_ptr;
	static char read_buf[100];

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return -1;
		} else if (read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;

	return 1;
}

ssize_t Readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t n, rc;
	char    c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c  == '\n')
				break;
		} else if (rc == 0) {
			*ptr = 0;
			return n - 1;
		} else
			return -1;
	}
	*ptr  = 0;

	return n;
}

