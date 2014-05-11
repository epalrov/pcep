/*
 * pce_client.c - PCE client application
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
#include <syslog.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

#include "pce_log.h"

#define PCE_SERVICE "4189"
#define PCE_HOSTNAME "localhost"

struct pce_client_data {
	int fd;
	struct addrinfo *addr;
	int debug;
};

/*
 * pce_client_send - PCE client send
 */
int pcep_client_send(struct pce_client_data *data, const void *buf, size_t count)
{
	int n, ret;
	const char *ptr;

	ptr = buf;
	for (ret = 0; ret < count; ) {
		n = write(data->fd, ptr, count - ret);
		if (n <= 0) {
			if (n == -1 && errno == EINTR)
				continue; /* interrupted, restart write() */
			else
				return -1; /* some other error */
		}
		ret += n;
		ptr += n;
	}
	return ret;
}

/*
 * pce_client_session - PCE client dummy session
 */
static int pce_client_session(struct pce_client_data *data)
{
	const char pcep_open[] = {
		0x20, 0x01, 0x00, 0x0C,
		0x01, 0x10, 0x00, 0x08,
		0x20, 0x00, 0x00, 0x00,
	};
	const char pcep_keepalive[] = {
		0x20, 0x02, 0x00, 0x04,
	};
	const char pcep_close[] = {
		0x20, 0x07, 0x00, 0x0C,
		0x0F, 0x10, 0x00, 0x08,
		0x00, 0x00, 0x00, 0x00,
	};

	pcep_client_send(data, pcep_open, sizeof(pcep_open));
	sleep(1);
	pcep_client_send(data, pcep_keepalive, sizeof(pcep_keepalive));
	sleep(2);
	pcep_client_send(data, pcep_close, sizeof(pcep_close));
	sleep(3);

	return 0;
}

/*
 * pce_client_init - PCE client initialization
 */
int pce_client_init(struct pce_client_data *data)
{
	int err;
	struct addrinfo *addr;

	pce_log(LOG_DEBUG, "starting PCE client ...\n");

	/* try to connect to the PCE server address(es) */
	for (addr = data->addr; addr != NULL; addr = addr->ai_next) {
		/* create connecting socket */
		data->fd = socket(addr->ai_family, addr->ai_socktype,
			addr->ai_protocol);
		if (data->fd == -1)
			continue;

		/* try to connect */
		err = connect(data->fd, addr->ai_addr, addr->ai_addrlen);
		if (err)
			close(data->fd);
		else
			break;
	}

	/* check if any address succeded */
	if (!addr || err) {
		pce_log(LOG_ERR, "can't connect to PCE server\n");
		goto out1;
	}

	/* dummy session */
	err = pce_client_session(data);

out2:
	pce_log(LOG_DEBUG, "closing PCE client ...\n");
	close(data->fd);
out1:
	return err;
}

static void pce_client_usage(FILE * out)
{
	static const char usage_str[] =
		("Usage:                                                \n"
		"  pce client [options]                               \n\n"
		"Options:                                               \n"
		"  -a | --address    PCE server address/hostname        \n"
		"  -p | --port       PCE server port/service            \n"
		"  -d | --debug      PCE client debug mode              \n"
		"  -v | --version    show the program version and exit  \n"
		"  -h | --help       show this help and exit          \n\n"
		"Examples:                                              \n"
		"  pce client -d --address localhost --port 4189      \n\n");

	fprintf(out, "%s", usage_str);
	fflush(out);
	return;
}

static void pce_client_version(FILE * out)
{
	static const char prog_str[] = "pce client";
	static const char ver_str[] = "1.1";
	static const char author_str[] = "Paolo Rovelli";

	fprintf(out, "%s %s written by %s\n", prog_str, ver_str, author_str);
	fflush(out);
	return;
}

static const struct option pce_client_options[] = {
	{"addr", required_argument, NULL, 'a'},
	{"port", required_argument, NULL, 'p'},
	{"debug", no_argument, NULL, 'd'},
	{"version", no_argument, NULL, 'v'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0}
};

int pce_client_main(int argc, char *argv[])
{
	int err, opt;
	int debug = 0;
	char *port = PCE_SERVICE;
	char *addr = PCE_HOSTNAME;
	struct addrinfo hints;
	struct pce_client_data *data;

	/* parse PCE client command line options */
	while ((opt = getopt_long(argc, argv, "da:p:vh", pce_client_options,
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
			pce_client_version(stdout);
			exit(EXIT_SUCCESS);
		case 'h':
			pce_client_usage(stdout);
			exit(EXIT_SUCCESS);
		default:
			pce_client_usage(stderr);
			exit(EXIT_FAILURE);
		}
	}

	/* init PCE logger */
	pce_log_open(debug ? LOG_PERROR : LOG_PID);
	pce_log_level(debug ? LOG_DEBUG : LOG_ERR);

	/* allocate PCE client data */
	data = calloc(sizeof(*data), 1);
	if (!data) {
		err = ENOMEM;
		pce_log(LOG_ERR, "failed to get memory\n");
		goto out1;
	}
	data->debug = debug;

	/* obtain address(es) structure matching host/service */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;
	err = getaddrinfo(addr, port, &hints, &data->addr);
	if (err) {
		pce_log(LOG_ERR, "failed getaddrinfo: %s\n", gai_strerror(err));
		goto out2;
	}

	/* init PCE client */
	err = pce_client_init(data);

	/* free PCE client resources */
	freeaddrinfo(data->addr);
out2:
	free(data);
out1:
	pce_log_close();
out:
	exit(err ? EXIT_FAILURE : EXIT_SUCCESS);
}
