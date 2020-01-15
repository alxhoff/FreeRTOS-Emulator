
// Alex Hoffman 2019
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

#define MQ_MAX_NAME_LEN 256
#define MQ_MAXMSG 256
#define MQ_MSGSIZE 256

#define CHECK(x)                                                               \
	do {                                                                   \
		if (!(x)) {                                                    \
			fprintf(stderr, "%s:%d: ", __func__, __LINE__);        \
			perror(#x);                                            \
			exit(-1);                                              \
		}                                                              \
	} while (0)

typedef enum {
	NONE = 0,
	TCP,
	UDP,
	MSG_QUEUE,
	SERIAL,
	NO_OF_CONN_TYPES
} aIO_conn_e;

typedef struct {
	int fd;
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
	ssize_t buffer_size;
	char *buffer;

	pthread_t thread;

	void (*callback)(ssize_t, char *, void *);
	void *args;
	struct aIO *next;

	pthread_mutex_t lock;
} aIO_t;

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
		case TCP:
		case UDP:
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

void aIODeinit(void)
{
	aIO_t *iterator, *del;

	if (head.next)
		for (iterator = head.next; iterator;) {
			del = iterator;
			iterator = iterator->next;
			switch (del->type) {
			case UDP:
			case TCP:
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
}

aIO_t *createAsyncIO(aIO_conn_e type, ssize_t buffer_size,
		     void (*callback)(ssize_t, char *, void *), void *args)
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

static void aIOMQSigHandler(int signal, siginfo_t *info, void *context)
{
    printf("In MQ sighandler: %d\n", getpid());
	aIO_t *conn = (aIO_t *)info->si_value.sival_ptr;

	ssize_t bytes_read = mq_receive(conn->attr.mq.fd, conn->buffer,
					conn->buffer_size, NULL);

	if (bytes_read > 0)
		(conn->callback)(bytes_read, conn->buffer, conn->args);

    /** reprime MQ notifications */
	CHECK(!mq_notify(conn->attr.mq.fd, &conn->attr.mq.ev));
}

int aIOMessageQueuePut(char *mq_name, char* buffer)
{
    mqd_t mq;
    char *full_name = calloc(strlen(mq_name) + 2, sizeof(char));
    strcpy(full_name + 1, mq_name);
    full_name[0] = '/';

    mq = mq_open(full_name, O_WRONLY);

    if ((mqd_t)-1 == mq){
        printf("Unable to open MQ '%s'\n", full_name);
        return -1;
    }

    CHECK(0 <= mq_send(mq, buffer, strlen(buffer), 0));

    CHECK((mqd_t)-1 != mq_close(mq));

    return 0;
}

aIO_handle_t aIOOpenMessageQueue(char *name, long max_msg_num,
				 long max_msg_size,
				 void (*callback)(ssize_t, char *, void *),
				 void *args)
{
	aIO_t *conn = getLastConnection();

	conn->next = createAsyncIO(MSG_QUEUE, max_msg_size, callback, args);
	CHECK(conn->next);

    pthread_mutex_lock(&conn->next->lock);

	conn = conn->next;

	aIO_mq_t *mq = &conn->attr.mq;

    size_t str_len = strlen(name) <= MQ_MAX_NAME_LEN ? strlen(name) : MQ_MAX_NAME_LEN;

	mq->name = (char *)malloc(sizeof(char) * (str_len + 2));
    printf("Name string for %s allocated at %p\n", name, mq->name);
	CHECK(mq->name);
	strncpy(mq->name + 1, name, str_len);
    mq->name[0] = '/';

	struct mq_attr attr;
	struct sigaction sa;
	union sigval sv;

	/** Attributes of MQ used in mq_open*/
	attr.mq_maxmsg = max_msg_num < MQ_MAXMSG ? max_msg_num : MQ_MAXMSG;
	attr.mq_msgsize = max_msg_size < MQ_MSGSIZE ? max_msg_size : MQ_MSGSIZE;
	attr.mq_curmsgs = 0;

	/** sigval struct that is passed to handler. sival_ptr is used to pass pointer  */
	/**     to structure containing MQ information */
	sv.sival_ptr = conn;

	/** Create MQ */
	CHECK(-1 != (mq->fd = mq_open(mq->name, O_CREAT | O_RDONLY | O_NONBLOCK, 0644,
				&attr)));

	/** Handler for SIGIO of MQ */
	sa.sa_flags = SA_SIGINFO; // Use sa_sigaction instead of sa_handler
	sa.sa_sigaction = aIOMQSigHandler; // Handler function
	sigfillset(&sa.sa_mask);
	sigdelset(&sa.sa_mask, SIGIO);
	CHECK(!sigaction(SIGIO, &sa, NULL));
    CHECK(-1 != fcntl(mq->fd, F_SETOWN, getpid()));

	/** sigevent needed to enable to passing of si_value to the handler */
	conn->attr.mq.ev.sigev_notify =
		SIGEV_SIGNAL; //Enables passing of si_value to handler
    mq->ev.sigev_signo = SIGIO;
    mq->ev.sigev_value = sv;
    /** used by SIGEV_THREAD */
    mq->ev.sigev_notify_function = NULL;
    mq->ev.sigev_notify_attributes = NULL;

    CHECK(!mq_notify(mq->fd, &mq->ev));

    pthread_mutex_unlock(&conn->lock);

    printf("MQ '%s' opened and notified\n", name);

	return (aIO_handle_t)conn;
}

static void aIOUDPSigHandler(int signal, siginfo_t *info, void *context)
{
	printf("In UDP handler: %d\n", getpid());
	ssize_t read_size;
	int server_fd = info->si_fd;
	aIO_t *conn = findConnection(UDP, &server_fd);

	CHECK(conn);

	pthread_mutex_lock(&conn->lock);

	while ((read_size = recv(server_fd, conn->buffer, conn->buffer_size,
				 0)) > 0) {
		conn->buffer[(read_size - 1) <= conn->buffer_size ?
				     (read_size - 1) :
				     conn->buffer_size] = '\0';
		printf("Buffer: %s\n", conn->buffer);
		(conn->callback)(read_size, conn->buffer, conn->args);
	}

	pthread_mutex_unlock(&conn->lock);
}

aIO_handle_t aIOOpenUDPSocket(char *s_addr, in_port_t port, ssize_t buffer_size,
			      void (*callback)(ssize_t, char *, void *),
			      void *args)
{
	aIO_t *conn = getLastConnection();

	conn->next = createAsyncIO(UDP, buffer_size, callback, args);

	pthread_mutex_lock(&conn->next->lock);

	aIO_socket_t *s_udp = &conn->next->attr.socket;

	s_udp->addr.sin_family = AF_INET;
	s_udp->addr.sin_addr.s_addr = s_addr ? inet_addr(s_addr) : INADDR_ANY;
	s_udp->addr.sin_port = htons(port);

	s_udp->fd = socket(AF_INET, SOCK_DGRAM, 0);
	CHECK(s_udp->fd);

	struct sigaction act = { 0 };
	int fs;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = aIOUDPSigHandler;
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

/** typedef struct { */
/**     int client_fd; */
/**     ssize_t buffer_size; */
/**  */
/**     void (*callback)(ssize_t, char *, void *); */
/**     void *args; */
/** } aIO_tcp_client; */
/**  */
/** void *aIOTCPHandler(void *conn) */
/** { */
/**     printf("TCP handler\n"); */
/**     ssize_t read_size; */
/**     aIO_tcp_client *client = (aIO_tcp_client *)conn; */
/**     int client_fd = client->client_fd; */
/**     char *buffer = malloc(sizeof(char) * client->buffer_size); */
/**     CHECK(buffer); */
/**  */
/**     while ((read_size = recv(client_fd, buffer, client->buffer_size, 0))) */
/**         (client->callback)(read_size, buffer, client->args); */
/**  */
/**     free(buffer); */
/**     free((aIO_tcp_client *)conn); */
/**     close(client_fd); */
/**  */
/**     return NULL; */
/** } */
/**  */
/** static void aIOTCPSigHandler(int signal, siginfo_t *info, void *context) */
/** { */
/**     struct sockaddr_in client; */
/**     int client_fd, server_fd = info->si_fd; */
/**     socklen_t client_size = sizeof(struct sockaddr_in); */
/**     aIO_t *conn = findConnection(TCP, &server_fd); */
/**  */
/**     CHECK(conn); */
/**  */
/**     pthread_mutex_lock(&conn->lock); */
/**  */
/**     while ((client_fd = accept(server_fd, (struct sockaddr *)&client, */
/**                    &client_size)) > 0) { */
/**         pthread_t handler_thread; */
/**         aIO_tcp_client *new_client = */
/**             (aIO_tcp_client *)malloc(sizeof(aIO_tcp_client)); */
/**         new_client->client_fd = client_fd; */
/**         new_client->buffer_size = conn->buffer_size; */
/**         new_client->callback = conn->callback; */
/**         new_client->args = conn->args; */
/**  */
/**         CHECK(!pthread_create(&handler_thread, NULL, aIOTCPHandler, */
/**                       (void *)new_client)); */
/**     } */
/**  */
/**     pthread_mutex_unlock(&conn->lock); */
/** } */
/**  */
/** void *aIOOpenTCPSocketThread(void *s_tcp_fd) */
/** { */
/**     int fd = *(int *)s_tcp_fd; */
/**     CHECK(-1 != fcntl(fd, F_SETOWN, gettid())); */
/**  */
/**     pthread_mutex_lock(&aIO_quit_lock); */
/**     pthread_cond_wait(&aIO_quit_conn, &aIO_quit_lock); */
/**     pthread_mutex_unlock(&aIO_quit_lock); */
/**  */
/**     return NULL; */
/** } */
/**  */
/** aIO_handle_t aIOOpenTCPSocket(char *s_addr, in_port_t port, ssize_t buffer_size, */
/**                   void (*callback)(ssize_t, char *, void *), */
/**                   void *args) */
/** { */
/**     aIO_t *conn = getLastConnection(); */
/**  */
/**     conn->next = createAsyncIO(TCP, buffer_size, callback, args); */
/**  */
/**     pthread_mutex_lock(&conn->next->lock); */
/**  */
/**     aIO_socket_t *s_tcp = &conn->next->attr.socket; */
/**  */
/**     s_tcp->addr.sin_family = AF_INET; */
/**     s_tcp->addr.sin_addr.s_addr = s_addr ? inet_addr(s_addr) : INADDR_ANY; */
/**     s_tcp->addr.sin_port = htons(port); */
/**  */
/**     s_tcp->fd = socket(AF_INET, SOCK_STREAM, 0); */
/**     CHECK(s_tcp->fd); */
/**  */
/**     struct sigaction act = { 0 }; */
/**     int fs; */
/**  */
/**     act.sa_flags = SA_SIGINFO; */
/**     act.sa_sigaction = aIOTCPSigHandler; */
/**     sigfillset(&act.sa_mask); */
/**     sigdelset(&act.sa_mask, SIGIO); */
/**     CHECK(!sigaction(SIGIO, &act, NULL)); */
/**  */
/**     CHECK((fs = fcntl(s_tcp->fd, F_GETFL))); */
/**     fs |= O_ASYNC | O_NONBLOCK; */
/**     CHECK(-1 != fcntl(s_tcp->fd, F_SETFL, fs)); */
/**     fcntl(s_tcp->fd, F_SETSIG, SIGIO); */
/**     CHECK(-1 != fcntl(s_tcp->fd, F_SETOWN, getpid())); */
/**  */
/**     CHECK(!bind(s_tcp->fd, (struct sockaddr *)&s_tcp->addr, */
/**             sizeof(s_tcp->addr))); */
/**  */
/**     CHECK(!listen(s_tcp->fd, 1)); */
/**  */
/**     CHECK(!pthread_create(&conn->next->thread, NULL, aIOOpenTCPSocketThread, */
/**                   (void *)&conn->next->attr.socket.fd)); */
/**  */
/**     pthread_mutex_unlock(&conn->next->lock); */
/**  */
/**     return (aIO_handle_t)conn->next; */
/** } */
