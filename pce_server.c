/*
 * pce_server.c - PCE server application
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <syslog.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/timerfd.h>

#include "pce_log.h"
#include "pce_pidfile.h"
#include "pcep_msg.h"
#include "pcep_framer.h"

#define PCE_SERVICE "4189"
#define PCE_PIDFILE "/var/run/pce.pid"

struct pce_server_data {
	int cfd;
	int lfd;
	struct addrinfo *addr;
	int tfd;
	struct itimerspec tval;
	int debug;
};

#define PCEP_FD_SOCKET 0
#define PCEP_FD_TIMER 1
#define PCEP_FD_TOT 2

#define PCEP_MSG_CHUNK 9
static void pcep_session(struct pce_server_data *data)
{
	char *buf;
	char dump[80];
	uint64_t elaps;
	ssize_t count;
	struct pcep_msg_hdr *pcep_msg;
	struct pcep_framer *pcep_framer;
	struct pollfd fds[2];

	buf = malloc(PCEP_MSG_CHUNK);
	if (!buf) {
		pce_log(LOG_ERR, "failed to get memory\n");
		goto out;
	}

	/* create a PCEP message framer */
	pcep_framer = pcep_framer_create();
	if (pcep_framer == NULL) {
		pce_log(LOG_ERR, "failed to create PCEP framer\n");
		goto out1;
	}

	/* create a periodic one second timer */
	data->tval.it_interval.tv_sec = 1,
	data->tval.it_value.tv_sec = 1,
	data->tfd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (!data->tfd) {
		pce_log(LOG_ERR, "failed to create PCEP session timer\n");
		goto out2;
	}
	if (timerfd_settime(data->tfd, 0, &data->tval, NULL) != 0) {
		pce_log(LOG_ERR, "failed to set PCEP session timer\n");
		goto out3;
	}

	/* specify events of interest */
	fds[PCEP_FD_SOCKET].fd = data->cfd;
	fds[PCEP_FD_SOCKET].events = POLLIN;
	fcntl(data->cfd, F_SETFL, fcntl(data->cfd, F_GETFL) | O_NONBLOCK);
	fds[PCEP_FD_TIMER].fd = data->tfd;
	fds[PCEP_FD_TIMER].events = POLLIN;
	fcntl(data->tfd, F_SETFL, fcntl(data->tfd, F_GETFL) | O_NONBLOCK);
		
	while (1) {

		/* block on interested events */
		if (poll(fds, PCEP_FD_TOT, -1) < 0)
			break;

		/* socket data available */
		if (fds[PCEP_FD_SOCKET].revents & POLLIN) {

			/* read a message chunk from the incoming stream */
			do {
				count = read(data->cfd, buf, PCEP_MSG_CHUNK);
			} while (count == -1 && errno == EINTR);

			/* check for any (fatal) error */
			if (count == -1)
				break;
			/* check if the peer socket hunged-up */
			if (count == 0)
				break;

			/* feed the framer with the message chunk */
			pcep_framer_write(pcep_framer, buf, count);

			/* handle messages if any */
			while (pcep_msg = pcep_framer_read(pcep_framer)) {

				pcep_msg_hdr_dump(pcep_msg, dump, sizeof(dump));
				pce_log(LOG_DEBUG, "%s\n", dump);

				/* handle the message */
				// TODO pcep_handle_msg(pcep_session, pcep_msg);
				
				/* release the message memory */
				pcep_msg_free(pcep_msg);
			}
		}

		/* timer expired */
		if (fds[PCEP_FD_TIMER].revents & POLLIN) {

			/* read the expiration time */
			do {
				count = read(data->tfd, &elaps, sizeof(elaps));
			} while (count == -1 && errno == EINTR);

			/* any error? */
			if (count != sizeof(elaps))
				break;

			pce_log(LOG_DEBUG, "timer exipred, elapsed %d\n", elaps);

			/* handle the timer */
			// TODO pcep_handle_timer(pcep_session, elaps);
		}
	}

out3:
	close(data->tfd);
out2:
	pcep_framer_delete(pcep_framer);
out1:
	free(buf);
out:
	return;
}

/*
 * pce_server_init - PCE server initialization
 */
int pce_server_init(struct pce_server_data *data)
{
	int err, sock_opt;
	struct addrinfo *addr;

	pce_log(LOG_DEBUG, "starting PCE server ...\n");

	/* try to bind the PCE server to any address */
	for (addr = data->addr; addr != NULL; addr = addr->ai_next) {

		/* create listening socket */
		data->lfd = socket(addr->ai_family, addr->ai_socktype,
			addr->ai_protocol);
		if (data->lfd == -1)
			continue;

		/* allow address reseuse */
		sock_opt = 1;
		setsockopt(data->lfd, SOL_SOCKET, SO_REUSEADDR,
			(void *)&sock_opt, sizeof(sock_opt));

		/* try to bind to the retrieved address */
		err = bind(data->lfd, addr->ai_addr, addr->ai_addrlen);
		if (err)
			close(data->lfd);
		else
			break;
	}

	/* check if any address succeded */
	if (!addr || err) {
		pce_log(LOG_ERR, "can't bind PCE server to any address\n");
		goto out1;
	}

	/* put the PCE server listening */
	if (listen(data->lfd, 0) < 0) {
		pce_log(LOG_ERR, "failure in listen(): %s\n",
			strerror(errno));
		goto err2;
	}

	/* concurrent PCE server */
	while (1) {

		socklen_t cl_addrlen;
		struct sockaddr_storage cl_addr;
		char hbuf[NI_MAXHOST];
		char sbuf[NI_MAXSERV];

		/* accept connections from PCE clients */
		cl_addrlen = sizeof(cl_addr);
		data->cfd = accept(data->lfd, (struct sockaddr *)&cl_addr,
			&cl_addrlen);
		if (data->cfd < 0) {
			pce_log(LOG_ERR, "failure in accept(): %s\n",
				strerror(errno));
			continue;
		}
		err = getnameinfo((struct sockaddr *) &cl_addr, cl_addrlen,
			hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
			NI_NUMERICHOST | NI_NUMERICSERV);
		if (!err)
			pce_log(LOG_DEBUG, "accepted connection from "
				"'%s:%s'\n", hbuf, sbuf);
		else
			pce_log(LOG_DEBUG, "accepted connection from "
				"'unknown' host\n");

		/* create a new process to handle each session */
		switch (fork()) {
		case -1: /* error */
			pce_log(LOG_ERR, "failure in fork(): %s\n",
				strerror(errno));
			close(data->cfd);
			break;

		case 0: /* child */
			close(data->lfd);
			signal(SIGINT, SIG_DFL);
			signal(SIGTERM, SIG_DFL);
			signal(SIGCHLD, SIG_DFL);
			signal(SIGPIPE, SIG_IGN);
			pce_log(LOG_DEBUG, "starting PCEP session ...\n");
			pcep_session(data);
			pce_log(LOG_DEBUG, "closing PCEP session ...\n");
			close(data->cfd);
			exit(EXIT_SUCCESS);
			break;

		default: /* parent */
			close(data->cfd);
			continue;
		}
	}

	return;

err2:
	pce_log(LOG_DEBUG, "closing PCE server ...\n");
	close(data->lfd);
out1:
	return err;
}

static void pce_sigchld_handler(int signal)
{
	pid_t pid;
	int status;

	/* handle exit of more than one child */
	do {
		pid = waitpid(-1, &status, WNOHANG);
	} while (pid > 0);
}


static void pce_sigterm_handler(int signal)
{
	pce_pidfile_delete(PCE_PIDFILE);
	exit(EXIT_SUCCESS);
}

static void pce_server_usage(FILE * out)
{
	static const char usage_str[] =
		("Usage:                                                \n"
		"  pce server [options]                               \n\n"
		"Options:                                               \n"
		"  -a | --address    PCE server address                 \n"
		"  -p | --port       PCE server port                    \n"
		"  -d | --debug      PCE server debug mode              \n"
		"  -v | --version    show the program version and exit  \n"
		"  -h | --help       show this help and exit          \n\n"
		"Examples:                                              \n"
		"  pce server -d -p 4189                              \n\n");

	fprintf(out, "%s", usage_str);
	fflush(out);
	return;
}

static void pce_server_version(FILE * out)
{
	static const char prog_str[] = "pce server";
	static const char ver_str[] = "1.1";
	static const char author_str[] = "Paolo Rovelli";

	fprintf(out, "%s %s written by %s\n", prog_str, ver_str, author_str);
	fflush(out);
	return;
}

static const struct option pce_server_options[] = {
	{"addr", required_argument, NULL, 'a'},
	{"port", required_argument, NULL, 'p'},
	{"debug", no_argument, NULL, 'd'},
	{"version", no_argument, NULL, 'v'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0}
};

int pce_server_main(int argc, char *argv[])
{
	int err, opt;
	int debug = 0;
	int ppid = getpid();
	char *port = PCE_SERVICE;
	char *addr = NULL;
	struct addrinfo hints;
	struct pce_server_data *data;

	/* parse PCE server command line options */
	while ((opt = getopt_long(argc, argv, "da:p:vh", pce_server_options,
				NULL)) != -1) {
		switch (opt) {
		case 'd':
			debug = 1;
			break;
		case 'a':
			addr = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 'v':
			pce_server_version(stdout);
			exit(EXIT_SUCCESS);
		case 'h':
			pce_server_usage(stdout);
			exit(EXIT_SUCCESS);
		default:
			pce_server_usage(stderr);
			exit(EXIT_FAILURE);
		}
	}

	/* init PCE logger */
	pce_log_open(debug ? LOG_PERROR : LOG_PID);
	pce_log_level(debug ? LOG_DEBUG : LOG_ERR);

	/* allocate PCE server data */
	data = calloc(sizeof(*data), 1);
	if (!data) {
		err = ENOMEM;
		pce_log(LOG_ERR, "failed to get memory\n");
		goto out1;
	}
	data->debug = debug;

	/* obtain address(es) structure matching service */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
	err = getaddrinfo(addr, port, &hints, &data->addr);
	if (err || !data->addr) {
		pce_log(LOG_ERR, "failed getaddrinfo: %s\n", gai_strerror(err));
		goto out2;
	}

	/* daemonize */
	if (!data->debug) {

		/* pidfile already exists ? exit : fork */
		if (pce_pidfile_check(PCE_PIDFILE)) {
			exit(EXIT_FAILURE);
		} else {
			/* become background process */
			switch (fork()) {
			case 0: /* child */
				break;
			case -1: /* error */
				exit(EXIT_FAILURE);
			default: /* parent */
				exit(EXIT_SUCCESS);
			}

			/* create a new session ID */
			if (setsid() < 0)
				exit(EXIT_FAILURE);

			/* change the file mode mask */
			umask(0);

			/* change the current working directory */
			if ((chdir("/")) < 0)
				exit(EXIT_FAILURE);

			/* close the standard file descriptors */
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
		}
	}

	/* pidfile already exists ? exit : create */
	if (pce_pidfile_check(PCE_PIDFILE)) {
		if (getpid() != ppid)
			kill (ppid, SIGTERM);
		exit(EXIT_FAILURE);
	} else {
		if (!pce_pidfile_create(PCE_PIDFILE)) {
			/* can't create pidfile (fatal error, exit) */
			pce_log(LOG_ERR, "can't create PCE pidfile\n");
			if (getpid() != ppid)
				kill (ppid, SIGTERM);
			exit(EXIT_FAILURE);
		}
	}

	/* register PCE server signal handlers */
	signal(SIGINT, pce_sigterm_handler);
	signal(SIGTERM, pce_sigterm_handler);
	signal(SIGCHLD, pce_sigchld_handler);

	/* init PCE server */
	err = pce_server_init(data);

	/* free PCE server resources */
	freeaddrinfo(data->addr);
out2:
	free(data);
out1:
	pce_log_close();
out:
	exit(err ? EXIT_FAILURE : EXIT_SUCCESS);
}
