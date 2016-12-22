/* vim: ft=c ff=unix fenc=utf-8
 * file: src/evsock.c
 */
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <arpa/inet.h>

#include "evsock.h"

#define SADDR_MAX 47
void
saddr_char(char *str, size_t size, sa_family_t family, struct sockaddr *sa)
{
    char xhost[40];
    unsigned short port = 0;
    switch(family) {
    case AF_INET:
        inet_ntop(AF_INET, &((struct sockaddr_in*)sa)->sin_addr,
                xhost, sizeof(xhost));
        port = ntohs(((struct sockaddr_in*)sa)->sin_port);
        if (port) {
            snprintf(str, size, "%s:%u", xhost, (unsigned short)port);
        } else {
            snprintf(str, size, "%s", xhost);
        }
        break;
    case AF_INET6:
        inet_ntop(AF_INET6, &((struct sockaddr_in6*)sa)->sin6_addr,
                xhost, sizeof(xhost));
        port = ntohs(((struct sockaddr_in6*)sa)->sin6_port);
        if (port) {
            snprintf(str, size, "[%s]:%u", xhost, port);
        } else {
            snprintf(str, size, "%s", xhost);
        }
        break;
    default:
        snprintf(str, size, "[unknown fa: %d]", family);
        break;
    }
}

/* setup default values */
static void
evs_internal_desc_setup(TL_V, struct evs_desc *d)
{
	tlog_trace("(d=%p)", (void*)d);
	d->socket_block_size = 4098;
}

static void
evs_internal_signal_int_cb(struct ev_loop *loop, struct ev_signal *w, int revents)
{
	TL_X;

	struct evs *evm = (void*)(((char*)w) - offsetof(struct evs, sigint));

	tlog_trace("(loop=%p, w=%p [%p], revents=%d)",
			(void*)loop, (void*)w, (void*)evm, revents);

	ev_break(loop, EVBREAK_ALL);
}

static void
evs_internal_signal_pipe_cb(struct ev_loop *loop, struct ev_signal *w, int revents)
{
	TL_X;

	struct evs *evm = (void*)(((char*)w) - offsetof(struct evs, sigint));

	tlog_trace("(loop=%p, w=%p [%p], revents=%d)",
			(void*)loop, (void*)w, (void*)evm, revents);

}

struct evs *
evs_setup(TL_V, struct evs *evm, struct ev_loop *loop)
{
	struct mmp *mmp = NULL;

	tlog_trace("(evm=%p)", (void*)evm);

	if (!(mmp = mmp_create())) {
		tlog_error("mmp allocation failed: %s", strerror(errno));
		return NULL;
	}

	if (evm) {
		/* initialize passed struct */
		memset(evm, 0, sizeof(*evm));
	} else {
		/* allocate new */
		if (!(evm = mmp_calloc(mmp, sizeof(*evm)))) {
			tlog_error("calloc(%d) failed: %s",
					sizeof(*evm), strerror(errno));
			mmp_destroy(mmp);
			return NULL;
		}
	}

	evm->mmp = mmp;

	if (loop) {
		evm->loop = loop;
	} else {
		evm->loop = ev_default_loop(0);
		evm->allocated_loop = true;

		/* setup signal handler */
		ev_signal_init(&evm->sigint, evs_internal_signal_int_cb, SIGINT);
		ev_signal_init(&evm->sigpipe, evs_internal_signal_pipe_cb, SIGPIPE);
		ev_signal_start(evm->loop, &evm->sigint);
		ev_signal_start(evm->loop, &evm->sigpipe);
	}

	return evm;
}

void
evs_
/* TODO: ... */destroy(TL_V, struct evs *evm)
{
	struct mmp *mmp = NULL;
	struct ev_loop *loop = NULL;

	tlog_trace("(evm=%p)", (void*)evm);

	loop = (evm->allocated_loop ? evm->loop : NULL);
	mmp = evm->mmp;

	ev_signal_stop(evm->loop, &evm->sigint);
	ev_signal_stop(evm->loop, &evm->sigpipe);

	mmp_destroy(mmp);

	if (loop) {
		ev_loop_destroy(loop);
	}
}


static void
evs_desc_destroy(struct evs_desc *d)
{
	TL_X;

	tlog_trace("(d=%p)", (void*)d);

	ev_async_stop(d->evm->loop, &d->async);
	ev_io_stop(d->evm->loop, &d->io);

	if (d->fd != -1) {
		close(d->fd);
	}
}

static void
evs_internal_connection_io_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	TL_X;

	struct evs_desc *d = (void*)(((char*)w) - offsetof(struct evs_desc, io));
	/* alloca()'s alternative */
	char buffer[d->socket_block_size];
	ssize_t rval = 0;

	memset(buffer, 0u, d->socket_block_size);

	tlog_trace("(loop=%p, w=%p [%p], revents=%d)",
			(void*)loop, (void*)w, (void*)d, revents);

	if (revents & EV_READ) {
		rval = read(d->fd, buffer, d->socket_block_size);
		if (rval == -1) {
			if (errno == EAGAIN) {
				/* FIXME: ? */
			} else {
				if (d->error) {
					(*d->error)(d, EVS_READ, strerror(errno));
				}
				tlog_info("desc#%p read error: %s", (void*)d, strerror(errno));
			}
		} else if (rval == 0) {
			/* close */
			tlog_info("desc#%p disconnect", (void*)d);
			if (d->disconnect) {
				(*d->disconnect)(d, d->raddr);
			}
			mmp_free(d);
			return;
		} else if (d->read) {
			/* pass data */
			rval = (*d->read)(d, buffer, (size_t)rval);
			if (!rval) {
				/* close connection */
				if (d->disconnect) {
					(*d->disconnect)(d, d->raddr);
				}
				tlog_info("desc#%p read handler discard data, exclude handler",
						(void*)d);
				evs_set_busy(TL_A, d, EVS_READ);
			}
		} else {
			tlog_notice("desc#%p no read handler, close connection", (void*)d);
			if (d->disconnect) {
				(*d->disconnect)(d, d->raddr);
			}
			/* close */
			mmp_free(d);
			return;
		}
	}

	if (revents & EV_WRITE) {
		if (!d->write) {
			/* TODO */
			tlog_notice("desc#%p no write handler, close connection", (void*)d);
			mmp_free(d);
			return;
		}
		rval = (*d->write)(d, buffer, d->socket_block_size);
		if (!rval) {
			tlog_notice("desc#%p no write data, exclude handler", (void*)d);
			evs_set_busy(TL_A, d, EVS_WRITE);
		}
		rval = write(d->fd, buffer, (size_t)rval);
		if (rval == -1) {
			tlog_info("desc#%p write error: %s", (void*)d, strerror(errno));
			/* TODO: */
		} else if (rval == 0) {
			/* TODO */
		} else {
			/* TODO */
		}
	}
}

static void
evs_internal_connection_async_cb(struct ev_loop *loop, struct ev_async *w, int revents)
{
	TL_X;
	char xaddr[SADDR_MAX] = {0};
	char host[EVS_MAX_ADDRESS] = {0};
	char port[6] = "0";
	char *end = NULL;

	struct addrinfo hints = {0};
	struct addrinfo *result = NULL;
	struct addrinfo *rp = NULL;
	int rval = 0;

	struct evs_desc *d = (void*)(((char*)w) - offsetof(struct evs_desc, async));

	tlog_trace("(loop=%p, w=%p, revents=%d)",
			(void*)loop, (void*)w, revents);

	if (d->fd != -1) {
		tlog_notice("invalid fd for (re)connect: %d", d->fd);
		return;
	}

	if (!(end = strrchr(d->addr, ':'))) {
		tlog_notice("not port passed in addr: '%s', using default value: %s",
				d->addr, port);
		strncpy(host, d->addr, sizeof(host) - 1);
	} else {
		snprintf(host, sizeof(host), "%.*s", (int)(end - d->addr), d->addr);
		snprintf(port, sizeof(port), "%s", end + 1);
	}

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	rval = getaddrinfo(host, port, &hints, &result);
	if (rval != 0) {
		tlog_notice("getaddrinfo(%s, %s) failed: %s",
				host, port, gai_strerror(rval));
		/* TODO: delay reconnect... */
		return;
	}

	for (rp = result; rp; rp = rp->ai_next) {
		if (d->fd != -1) {
			close(d->fd);
		}

		saddr_char(xaddr, sizeof(xaddr), rp->ai_family, rp->ai_addr);
		/* allocate new socket */
		d->fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (d->fd == -1) {
			tlog_notice("socket(%d, %d, %d) for addr '%s' failed: %s",
					rp->ai_family, rp->ai_socktype, rp->ai_protocol,
					xaddr, strerror(errno));
			continue;
		}

		if (connect(d->fd, rp->ai_addr, rp->ai_addrlen) != 0) {
			tlog_notice("connect(%s) failed: %s", xaddr, strerror(errno));
			continue;
		}

		break;
	}

	freeaddrinfo(result);

	/* onfail cleanup */
	if (!rp) {
		if (d->fd != -1) {
			close(d->fd);
			d->fd = -1;
		}
		/* TODO: delay reconnect */
	} else {
		snprintf(d->raddr, sizeof(d->raddr), "%s", xaddr);
		tlog_info("connected to: %s (%s)", d->addr, d->raddr);
		/* call event */
		if (d->connect) {
			(*d->connect)(d, d->raddr);
		}
		/* TODO: start events */
		ev_io_init(&d->io, evs_internal_connection_io_cb, d->fd,
				(((d->read) ? EV_READ : EV_NONE) | ((d->write) ? EV_WRITE : EV_NONE)));
		ev_io_start(d->evm->loop, &d->io);
	}

}

static void
evs_internal_acception_async_cb(struct ev_loop *loop, struct ev_async *w, int revents)
{
	TL_X;

	struct evs_desc *d = (void*)(((char*)w) - offsetof(struct evs_desc, async));

	tlog_trace("(loop=%p, w=%p [%p], revents=%d)",
			(void*)loop, (void*)w, (void*)d, revents);
}

static void
evs_internal_acception_io_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	TL_X;

	struct evs_desc *d = (void*)(((char*)w) - offsetof(struct evs_desc, async));

	tlog_trace("(loop=%p, w=%p [%p], revents=%d)",
			(void*)loop, (void*)w, (void*)d, revents);

}

static void
evs_internal_bind_accept_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	TL_X;
	char xaddr[SADDR_MAX] = {0};
	struct sockaddr_storage sa = {0};
	socklen_t sl = sizeof(sa);
	int fd = -1;
	struct evs_desc *nd = NULL;
	struct evs_desc *d = (void*)(((char*)w) - offsetof(struct evs_desc, io));

	tlog_trace("(loop=%p, w=%p [%p], revents=%d)",
			(void*)loop, (void*)w, (void*)d, revents);

	if (!(revents & EV_READ)) {
		tlog_warn("called without EV_READ flag", NULL);
		return;
	}

	fd = accept(d->fd, (struct sockaddr*)&sa, &sl);
	if (fd == -1) {
		tlog_notice("accept(%s) failed: %s", d->addr, strerror(errno));
		return;
	}

	saddr_char(xaddr, sizeof(xaddr), sa.ss_family, (struct sockaddr*)&sa);

	if (!d->accept) {
		tlog_notice("accept(%s) from %s canceled: no accept function",
				d->addr, xaddr);
		close(fd);
		return;
	}

	if (!(nd = mmp_calloc(d->evm->mmp, sizeof(*nd)))) {
		tlog_error("accept(%s) from %s canceled: calloc(%d) failed: %s",
				d->addr, xaddr,
				sizeof(*d), strerror(errno));
		close(fd);
		return;
	}

	strncat(nd->addr, d->addr, sizeof(d->addr) - 1);
	strncat(nd->raddr, xaddr, sizeof(d->raddr) - 1);
	nd->fd = fd;
	nd->evm = d->evm;
	nd->type = EVS_ACCEPTION;
	evs_internal_desc_setup(TL_A, nd);

	(*d->accept)(d, nd, xaddr);

	/* close acception when write or read handlers not defined */
	if (!d->read && !d->write) {
		tlog_info("accept(%s) from %s canceled: "
			   "read or write handlers not defined",
			   d->addr, xaddr);
		close(fd);
		mmp_free(nd);
		return;
	}

	/* continue initialize structure */
	mmp_modify(nd->evm->mmp, (void*)nd, (void(*)())evs_desc_destroy);

	tlog_info("accept(%s) from %s", d->addr, xaddr);

	/* start event */
	ev_async_init(&nd->async, evs_internal_acception_async_cb);
	ev_async_start(nd->evm->loop, &nd->async);

	ev_io_init(&nd->io, evs_internal_acception_io_cb, nd->fd,
			(((d->read) ? EV_READ : EV_NONE) | ((d->write) ? EV_WRITE : EV_NONE)));
	ev_io_start(nd->evm->loop, &nd->io);
}

static void
evs_internal_bind_cb(struct ev_loop *loop, struct ev_async *w, int revents)
{
	TL_X;
	char xaddr[SADDR_MAX] = {0};
	char host[EVS_MAX_ADDRESS] = {0};
	char port[6] = "0";
	char *end = NULL;

	struct addrinfo hints = {0};
	struct addrinfo *result = NULL;
	struct addrinfo *rp = NULL;
	int rval = 0;

	struct evs_desc *d = (void*)(((char*)w) - offsetof(struct evs_desc, async));

	tlog_trace("(loop=%p, w=%p [%p], revents=%d)",
			(void*)loop, (void*)w, (void*)d, revents);

	if (d->fd != -1) {
		tlog_notice("invalid fd for (re)bind: %d", d->fd);
		return;
	}

	if (!(end = strrchr(d->addr, ':'))) {
		tlog_notice("not port passed in addr: '%s', using default value: %s",
				d->addr, port);
		strncpy(host, d->addr, sizeof(host) - 1);
	} else {
		snprintf(host, sizeof(host), "%.*s", (int)(end - d->addr), d->addr);
		snprintf(port, sizeof(port), "%s", end + 1);
	}

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	/* list addresses */
	rval = getaddrinfo(host, port, &hints, &result);
	if (rval != 0) {
		tlog_notice("getaddrinfo(%s, %s) [%s] failed: %s",
				host, port, d->addr, gai_strerror(rval));
		/* TODO: delay... */
		return;
	}

	for (rp = result; rp; rp = rp->ai_next) {
		/* prevent leaks */
		if (d->fd != -1) {
			close(d->fd);
		}

		saddr_char(xaddr, sizeof(xaddr), rp->ai_family, rp->ai_addr);
		/* allocate new socket */
		d->fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (d->fd == -1) {
			tlog_notice("socket(%d, %d, %d) for addr '%s' failed: %s",
					rp->ai_family, rp->ai_socktype, rp->ai_protocol,
					xaddr, strerror(errno));
			continue;
		}

		setsockopt(d->fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

		if ((rval = bind(d->fd, rp->ai_addr, rp->ai_addrlen)) != 0) {
			tlog_notice("bind(%s) failed: %s", xaddr, strerror(errno));
			continue;
		}

		if (listen(d->fd, 10) != 0) {
			tlog_notice("listen(%s) failed: %s", xaddr, strerror(errno));
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	/* onfail cleanup */
	if (!rp) {
		if (d->fd != -1) {
			close(d->fd);
			d->fd = -1;
		}
		/* TODO: delay */
	} else {
		snprintf(d->raddr, sizeof(d->raddr), "%s", xaddr);
		tlog_info("listening: %s (%s)", d->addr, d->raddr);
		/* start events */
		ev_io_init(&d->io, evs_internal_bind_accept_cb, d->fd, EV_READ);
		ev_io_start(d->evm->loop, &d->io);
	}
}

struct evs_desc *
evs_bind(TL_V, struct evs *evm, const char *address, evs_accept_cb_t event_cb)
{
	struct evs_desc *d = NULL;

	tlog_trace("(evm=%p, address=%p [%s], event_cb=%p)",
			(void*)evm, (void*)address, (address ? address : ""), (void*)((uintptr_t)event_cb));

	/* allocate */
	if (!(d = mmp_calloc(evm->mmp, sizeof(*d)))) {
		tlog_error("calloc(%d) failed: %s", sizeof(*d), strerror(errno));
		return NULL;
	}

	strncat(d->addr, address, sizeof(d->addr) - 1);
	d->fd = -1;
	d->evm = evm;
	d->type = EVS_SERVER;
	d->accept = event_cb;
	evs_internal_desc_setup(TL_A, d);

	mmp_modify(evm->mmp, (void*)d, (void(*)())evs_desc_destroy);

	/* init events */
	ev_async_init(&d->async, evs_internal_bind_cb);
	ev_async_start(d->evm->loop, &d->async);

	/* begin event */
	ev_async_send(d->evm->loop, &d->async);
	return d;
}

struct evs_desc *
evs_connect(TL_V, struct evs *evm, const char *address, evs_connect_cb_t event_cb)
{
	struct evs_desc *d = NULL;

	tlog_trace("(evm=%p, address=%p [%s], event_cb=%p)",
			(void*)evm, (void*)address, (address ? address : ""), (void*)((uintptr_t)event_cb));

	if (!(d = mmp_calloc(evm->mmp, sizeof(*d)))) {
		tlog_error("calloc(%d) faled: %s", sizeof(*d), strerror(errno));
		return NULL;
	}

	strncat(d->addr, address, sizeof(d->addr) - 1);
	d->fd = -1;
	d->evm = evm;
	d->type = EVS_CONNECTION;
	d->connect = event_cb;
	evs_internal_desc_setup(TL_A, d);

	mmp_modify(evm->mmp, (void*)d, (void(*)())evs_desc_destroy);

	/* init */
	ev_async_init(&d->async, evs_internal_connection_async_cb);
	ev_async_start(evm->loop, &d->async);

	ev_async_send(evm->loop, &d->async);
	return d;
}

bool
evs_set_event(TL_V, struct evs_desc *d, enum evs_event e, evs_event_cb_t event_cb)
{
	evs_event_cb_t old_event_cb = NULL;
	const char *pt = NULL;

	tlog_trace("(d=%p, event_cb=%p)",
			(void*)d, (void*)((uintptr_t)event_cb));

	switch (e) {
		case EVS_ACCEPT:
			pt = "ACCEPT";
			old_event_cb = (evs_event_cb_t)d->accept;
			d->accept = (evs_accept_cb_t)event_cb;
			break;
		case EVS_CONNECT:
			pt = "CONNECT";
			old_event_cb = (evs_event_cb_t)d->connect;
			d->connect = (evs_connect_cb_t)event_cb;
			break;
		case EVS_DISCONNECT:
			pt = "DISCONNECT";
			old_event_cb = (evs_event_cb_t)d->disconnect;
			d->disconnect = (evs_connect_cb_t)event_cb;
			break;
		case EVS_READ:
			pt = "READ";
			old_event_cb = (evs_event_cb_t)d->read;
			d->read = (evs_read_cb_t)event_cb;
			break;
		case EVS_WRITE:
			pt = "WRITE";
			old_event_cb = (evs_event_cb_t)d->write;
			d->write = (evs_write_cb_t)event_cb;
			break;
		case EVS_ALARM:
			pt = "ALARM";
			/*
			old_event_cb = (evs_event_cb_t)d->alarm;
			d->alarm = (evs_alarm_cb_t)event_cb;
			*/
			break;
		case EVS_ERROR:
			pt = "ERROR";
			old_event_cb = (evs_event_cb_t)d->error;
			d->error = (evs_error_cb_t)event_cb;
			break;
	}

	tlog_debug("d=%p, set new event handler (type: %s) %p, old: %p",
			(void*)d, pt,
			(void*)(uintptr_t)event_cb, (void*)(uintptr_t)old_event_cb);

	return true;
}

bool
evs_set_ready(TL_V, struct evs_desc *d, enum evs_event t)
{
	tlog_trace("(d=%p, t=%d)", (void*)d, t);
	return false;
}

bool
evs_set_busy(TL_V, struct evs_desc *d, enum evs_event t)
{
	tlog_trace("(d=%p, t=%d)", (void*)d, t);
	return false;
}

void
evs_loop(TL_V, struct evs *evm)
{
	tlog_trace("(evm=%p)", (void*)evm);
	ev_run(evm->loop, 0);
}

