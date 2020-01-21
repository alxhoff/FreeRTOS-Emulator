/**
 * @file AsyncIO.c
 * @author Alex Hoffman
 * @date 20 January 2020
 * @brief A single file asyncronous UNIX communications library to perform
 * UDP, TCP and POSIX message queue communications.
 *
 * @verbatim
   ----------------------------------------------------------------------
    Copyright (C) Alexander Hoffman, 2019
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
@endverbatim
 */

#define _GNU_SOURCE

#include <mqueue.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include "AsyncIO.h"

#define CHECK(x)                                                               \
	do {                                                                   \
		if (!(x)) {                                                    \
			fprintf(stderr, "%s:%d: ", __func__, __LINE__);        \
			perror(#x);                                            \
			exit(-1);                                              \
		}                                                              \
	} while (0)

void *aIOTCPHandler(void *conn);

typedef enum {
	NONE = 0,
	SOCKET,
	MSG_QUEUE,
	SERIAL,
	NO_OF_CONN_TYPES
} aIO_conn_e;

typedef struct {
	int fd;
	aIO_socket_e type;
	struct sockaddr_in addr;
} aIO_socket_t;

typedef struct {
	mqd_t fd;
	char *name;
	struct sigevent ev;
} aIO_mq_t;

typedef struct {
	//TODO
} aIO_serial_t;

typedef union {
	aIO_socket_t socket;
	aIO_mq_t mq;
	aIO_serial_t tty;
} aIO_attr;

typedef struct aIO {
	aIO_conn_e type;

	aIO_attr attr;
	size_t buffer_size;
	char *buffer;

	pthread_t thread;

	void (*callback)(size_t, char *, void *);
	void *args;
	struct aIO *next;

	pthread_mutex_t lock;
} aIO_t;

typedef struct {
	int client_fd;
	size_t buffer_size;

	void (*callback)(size_t, char *, void *);
	void *args;
} aIO_tcp_client;

aIO_t head = { .type = NONE, .lock = PTHREAD_MUTEX_INITIALIZER };
pthread_cond_t aIO_quit_conn = PTHREAD_COND_INITIALIZER;
pthread_mutex_t aIO_quit_lock = PTHREAD_MUTEX_INITIALIZER;

aIO_t *getLastConnection(void)
{
	aIO_t *iterator;

	for (iterator = &head; iterator->next; iterator = iterator->next)
		;

	return iterator;
}

static aIO_t *findConnection(aIO_conn_e type, void *arg)
{
	aIO_t *prev = &head, *curr;

	pthread_mutex_lock(&prev->lock);
	while ((curr = prev->next) != NULL) {
		pthread_mutex_lock(&curr->lock);
		switch (type) {
		case SOCKET:
			if (curr->type == type)
				if (curr->attr.socket.fd == *(int *)arg) {
					pthread_mutex_unlock(&prev->lock);
					pthread_mutex_unlock(&curr->lock);
					return curr;
				}
			break;
		case MSG_QUEUE:
			//TODO
			return NULL;
		case SERIAL:
			//TODO
			return NULL;
		case NONE:
		default:
			return NULL;
		}
		pthread_mutex_unlock(&prev->lock);
		prev = curr;
	}
	pthread_mutex_unlock(&prev->lock);
	return NULL;
}

//TODO move this into functions that are calable such that connections can be
//closed during runtime

void aIOCloseConn(aIO_handle_t conn)
{
	CHECK(conn);

	aIO_t *del = (aIO_t *)conn;

	switch (del->type) {
	case SOCKET:
		printf("Deinit socket %d\n",
		       ntohs(del->attr.socket.addr.sin_port));
		CHECK(!close(del->attr.socket.fd));
		free(del->buffer);
		free(del);
		break;
	case MSG_QUEUE:
		printf("Deinit MQ %s\n", del->attr.mq.name);
		mq_close(del->attr.mq.fd);
		mq_unlink(del->attr.mq.name);
		free(del->attr.mq.name);
		free(del->buffer);
		free(del);
		break;
	default:
		break;
	}
}

void aIODeinit(void)
{
	aIO_t *iterator, *del;

	if (head.next)
		for (iterator = head.next; iterator;) {
			del = iterator;
			iterator = iterator->next;
			aIOCloseConn((aIO_handle_t)del);
		}
}

aIO_t *createAsyncIO(aIO_conn_e type, size_t buffer_size,
		     void (*callback)(size_t, char *, void *), void *args)
{
	aIO_t *ret = (aIO_t *)calloc(1, sizeof(aIO_t));

	CHECK(ret);

	ret->buffer_size = buffer_size;
	ret->buffer = (char *)malloc(ret->buffer_size * sizeof(char));
	CHECK(ret->buffer);

	ret->type = type;
	ret->callback = callback;
	ret->args = args;

	pthread_mutex_init(&ret->lock, NULL);

	return ret;
}

static void aIOMQSigHandler(union sigval sv)
{
	aIO_t *conn = (aIO_t *)sv.sival_ptr;

	ssize_t bytes_read = mq_receive(conn->attr.mq.fd, conn->buffer,
					conn->buffer_size, NULL);

	if (bytes_read > 0)
		(conn->callback)(bytes_read, conn->buffer, conn->args);

	/** reprime MQ notifications */
	CHECK(!mq_notify(conn->attr.mq.fd, &conn->attr.mq.ev));
}

int aIOMessageQueuePut(char *mq_name, char *buffer)
{
	mqd_t mq;
	char *full_name = calloc(strlen(mq_name) + 2, sizeof(char));
	strcpy(full_name + 1, mq_name);
	full_name[0] = '/';

	mq = mq_open(full_name, O_WRONLY);

	free(full_name);

	if ((mqd_t)-1 == mq) {
		printf("Unable to open MQ '%s'\n", mq_name);
		return -1;
	}

	if (-1 == mq_send(mq, buffer, strlen(buffer), 0))
		printf("Unable to send to MQ: %s, errno: %d\n", mq_name, errno);
	else
		printf("Sent to MQ: %s\n", mq_name);

	CHECK((mqd_t)-1 != mq_close(mq));

	return 0;
}

int aIOSocketPut(aIO_socket_e protocol, char *s_addr, in_port_t port,
		 char *buffer, size_t buffer_size)
{
	int fd;
	struct sockaddr_in server = { .sin_addr.s_addr =
					      s_addr ? inet_addr(s_addr) : 0,
				      .sin_family = AF_INET,
				      .sin_port = htons(port) };

	switch (protocol) {
	case TCP:
		if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			goto err_create_socket;
		break;
	case UDP:
		if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
			goto err_create_socket;
		break;
	default:
		return -1;
	}

	if (connect(fd, (struct sockaddr *)&server, sizeof(server)) < 0)
		goto err_connect;

	if (send(fd, buffer, buffer_size, 0) < 0)
		goto err_send;

	close(fd);

	return 0;

err_create_socket:
	fprintf(stderr, "Failed to create socket %s:%d\n", s_addr, port);
	return -1;

err_connect:
	fprintf(stderr, "Connecting to %s:%d failed\n", s_addr, port);
	return -1;

err_send:
	fprintf(stderr, "Sending to %s:%d failed\n", s_addr, port);
	return -1;
}

aIO_handle_t aIOOpenMessageQueue(char *name, long max_msg_num,
				 long max_msg_size,
				 void (*callback)(size_t, char *, void *),
				 void *args)
{
	aIO_t *conn = getLastConnection();

	conn->next = createAsyncIO(MSG_QUEUE, max_msg_size, callback, args);
	CHECK(conn->next);

	pthread_mutex_lock(&conn->next->lock);

	conn = conn->next;

	aIO_mq_t *mq = &conn->attr.mq;

	size_t str_len = strlen(name) <= MQ_MAX_NAME_LEN ? strlen(name) :
							   MQ_MAX_NAME_LEN;

	mq->name = (char *)malloc(sizeof(char) * (str_len + 2));
	CHECK(mq->name);
	strncpy(mq->name + 1, name, str_len);
	mq->name[0] = '/';

	struct mq_attr attr;
	union sigval sv;

	/** Attributes of MQ used in mq_open*/
	attr.mq_maxmsg = max_msg_num < MQ_MAXMSG ? max_msg_num : MQ_MAXMSG;
	attr.mq_msgsize = max_msg_size < MQ_MSGSIZE ? max_msg_size : MQ_MSGSIZE;
	attr.mq_curmsgs = 0;

	/** sigval struct that is passed to handler. sival_ptr is used to pass pointer  */
	/**     to structure containing MQ information */
	sv.sival_ptr = conn;

	/** Create MQ */
	CHECK(-1 != (mq->fd = mq_open(mq->name, O_CREAT | O_RDONLY | O_NONBLOCK,
				      0644, &attr)));

	/** sigevent needed to enable to passing of si_value to the handler */
	conn->attr.mq.ev.sigev_notify = SIGEV_THREAD;
	mq->ev.sigev_signo = SIGIO;
	mq->ev.sigev_value = sv;
	/** used by SIGEV_THREAD */
	mq->ev.sigev_notify_function = aIOMQSigHandler;
	mq->ev.sigev_notify_attributes = NULL;

	CHECK(!mq_notify(mq->fd, &mq->ev));

	pthread_mutex_unlock(&conn->lock);

	printf("MQ '%s' opened and notified\n", name);

	return (aIO_handle_t)conn;
}

static void aIOSocketSigHandler(int signal, siginfo_t *info, void *context)
{
	ssize_t read_size;
	int server_fd = info->si_fd;
	aIO_t *conn = findConnection(SOCKET, &server_fd);

	CHECK(conn);

	pthread_mutex_lock(&conn->lock);

	switch (conn->attr.socket.type) {
	case UDP:
		while ((read_size = recv(server_fd, conn->buffer,
					 conn->buffer_size, 0)) > 0) {
			conn->buffer[read_size <= conn->buffer_size ?
					     read_size :
					     conn->buffer_size] = '\0';
            if (conn->callback)
			(conn->callback)(read_size, conn->buffer, conn->args);
		}
		break;
	case TCP: {
		int client_fd;
		struct sockaddr_in client;
		socklen_t client_size = sizeof(struct sockaddr_in);
		while ((client_fd =
				accept(server_fd, (struct sockaddr *)&client,
				       &client_size)) > 0) {
			pthread_t handler_thread;
			aIO_tcp_client *new_client = (aIO_tcp_client *)malloc(
				sizeof(aIO_tcp_client));
			new_client->client_fd = client_fd;
			new_client->buffer_size = conn->buffer_size;
			new_client->callback = conn->callback;
			new_client->args = conn->args;

			CHECK(!pthread_create(&handler_thread, NULL,
					      *aIOTCPHandler,
					      (void *)new_client));
		}
	} break;
	default:
		break;
	}

	pthread_mutex_unlock(&conn->lock);
}

aIO_handle_t aIOOpenUDPSocket(char *s_addr, in_port_t port, size_t buffer_size,
			      void (*callback)(size_t, char *, void *),
			      void *args)
{
	aIO_t *conn = getLastConnection();

	conn->next = createAsyncIO(SOCKET, buffer_size, callback, args);
	CHECK(conn->next);

	conn->next->attr.socket.type = UDP;

	pthread_mutex_lock(&conn->next->lock);

	aIO_socket_t *s_udp = &conn->next->attr.socket;

	s_udp->addr.sin_family = AF_INET;
	s_udp->addr.sin_addr.s_addr = s_addr ? inet_addr(s_addr) : INADDR_ANY;
	s_udp->addr.sin_port = htons(port);

	s_udp->fd = socket(AF_INET, SOCK_DGRAM, 0);
	CHECK(s_udp->fd);

	printf("Opened socket on port %d with FD: %d\n", port, s_udp->fd);

	struct sigaction act = { 0 };
	int fs;

	act.sa_flags = SA_SIGINFO | SA_RESTART;
	act.sa_sigaction = aIOSocketSigHandler;
	sigfillset(&act.sa_mask);
	sigdelset(&act.sa_mask, SIGIO);
	CHECK(!sigaction(SIGIO, &act, NULL));

	CHECK((fs = fcntl(s_udp->fd, F_GETFL)));
	fs |= O_ASYNC | O_NONBLOCK;
	CHECK(-1 != fcntl(s_udp->fd, F_SETFL, fs));
	fcntl(s_udp->fd, F_SETSIG, SIGIO);
	CHECK(-1 != fcntl(s_udp->fd, F_SETOWN, getpid()));

	CHECK(!bind(s_udp->fd, (struct sockaddr *)&s_udp->addr,
		    sizeof(s_udp->addr)));

	pthread_mutex_unlock(&conn->next->lock);

	return (aIO_handle_t)conn->next;
}

void *aIOTCPHandler(void *conn)
{
	ssize_t read_size;
	aIO_tcp_client *client = (aIO_tcp_client *)conn;
	int client_fd = client->client_fd;
	char *buffer = malloc(sizeof(char) * client->buffer_size);
	CHECK(buffer);

	while ((read_size = recv(client_fd, buffer, client->buffer_size, 0)))
        if(client->callback)
		(client->callback)(read_size, buffer, client->args);

	close(client_fd);
	free(buffer);
	free((aIO_tcp_client *)conn);

	return NULL;
}

aIO_handle_t aIOOpenTCPSocket(char *s_addr, in_port_t port, size_t buffer_size,
			      void (*callback)(size_t, char *, void *),
			      void *args)
{
	aIO_t *conn = getLastConnection();

	conn->next = createAsyncIO(SOCKET, buffer_size, callback, args);
	CHECK(conn->next);
	conn->next->attr.socket.type = TCP;

	pthread_mutex_lock(&conn->next->lock);

	aIO_socket_t *s_tcp = &conn->next->attr.socket;

	s_tcp->addr.sin_family = AF_INET;
	s_tcp->addr.sin_addr.s_addr = s_addr ? inet_addr(s_addr) : INADDR_ANY;
	s_tcp->addr.sin_port = htons(port);

	s_tcp->fd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK(s_tcp->fd);

	printf("Opened socket on port %d with FD: %d\n", port, s_tcp->fd);

	struct sigaction act = { 0 };
	int fs;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = aIOSocketSigHandler;
	sigfillset(&act.sa_mask);
	sigdelset(&act.sa_mask, SIGIO);
	CHECK(!sigaction(SIGIO, &act, NULL));

	CHECK((fs = fcntl(s_tcp->fd, F_GETFL)));
	fs |= O_ASYNC | O_NONBLOCK;
	CHECK(-1 != fcntl(s_tcp->fd, F_SETFL, fs));
	fcntl(s_tcp->fd, F_SETSIG, SIGIO);
	CHECK(-1 != fcntl(s_tcp->fd, F_SETOWN, getpid()));

	CHECK(!bind(s_tcp->fd, (struct sockaddr *)&s_tcp->addr,
		    sizeof(s_tcp->addr)));

	CHECK(!listen(s_tcp->fd, 1));

	pthread_mutex_unlock(&conn->next->lock);

	return (aIO_handle_t)conn->next;
}
