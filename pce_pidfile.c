/*
 * pce_pidfile.c - PCE pidfile utilities
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "pce_pidfile.h"

int pce_pidfile_create(char *pathname)
{
	FILE *f;
	int fd;
	int pid;

	/* open or create the pidfile */
	fd = open(pathname, O_RDWR|O_CREAT|O_TRUNC,
		S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
	if (fd == -1)
		return 0;

	/* try to lock, otherwise the pidfile is held by another process */
	if (flock(fd, LOCK_EX|LOCK_NB) == -1)
		return 0;

	/* write the pid number to the pidfile */
	pid = getpid();
	f = fdopen(fd, "r+");
	fprintf(f,"%d\n", pid);
	fflush(f);

	/* unlock and close */
	flock(fd, LOCK_UN);

	/* close */
	close(fd);

	return pid;
}

int pce_pidfile_delete(char *pathname)
{
	return unlink(pathname);
}

int pce_pidfile_check(char *pathname)
{
	FILE *f;
	int pid;

	/* open the pidfile, if exists */
	f = fopen(pathname, "r");
	if (!f)
		return 0;

	/* read the pid number from the pidfile */
	if (fscanf(f,"%d", &pid) != 1)
		pid = 0;
	fclose(f);

	/* already holding the pidfile? */
	if (pid == getpid())
		return 0;

	/*
	 * use the null signal to test if a process with a specific process ID
	 * exists. If sending a null signal fails with the error ESRCH, then we
	 * know the process doesn’t exist.
	 */
	if (kill(pid, 0) && errno == ESRCH)
		return 0;

	return pid;
}
