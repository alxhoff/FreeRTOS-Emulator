#ifndef __ASYNCIO_H__
#define __ASYNCIO_H__

#include <netinet/in.h>

typedef void *aIO_handle_t;

aIO_handle_t aIOOpenUDPSocket(char *s_addr, in_port_t port, ssize_t buffer_size,
			      void (*callback)(ssize_t, char *, void *),
			      void *args);
aIO_handle_t aIOOpenTCPSocket(char *s_addr, in_port_t port, ssize_t buffer_size,
			      void (*callback)(ssize_t, char *, void *),
			      void *args);
void aIODeinit(void);

#endif
