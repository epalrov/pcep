/*
 * pce.c - PCE main application
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int pce_server_main(int argc, char **argv);
extern int pce_client_main(int argc, char **argv);

static void pce_usage(FILE * out)
{
	static const char usage_str[] =
		("Usage:                              \n"
		"  pce [command] [options]          \n\n"
		"Commands:                            \n"
		"  server    run pce server (PCE)     \n"
		"  client    run pce client (PCC)     \n"
		"  help      show this help and exit\n\n"
		"Examples:                            \n"
		"  pce server -d -p 4189              \n"
		"  pce client                       \n\n");

	fprintf(out, "%s", usage_str);
	fflush(out);

	return;
}

int main(int argc, char *argv[])
{
	if (argc > 1 && (strcmp(argv[1], "server") == 0)) {
		argc--;
		argv++;
		pce_server_main(argc, argv);
	} else if (argc > 1 && (strcmp(argv[1], "client") == 0)) {
		argc--;
		argv++;
		pce_client_main(argc, argv);
	} else if (argc > 1 && (strcmp(argv[1], "help") == 0)) {
		pce_usage(stderr);
		exit(EXIT_FAILURE);
	} else {
		pce_usage(stderr);
		exit(EXIT_FAILURE);
	}

	return 0;
}
