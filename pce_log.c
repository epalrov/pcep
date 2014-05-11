/*
 * pce_log.c - PCE logger
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "pce_log.h"

static int pce_log_stderr = 0;
static int pce_log_priority = LOG_WARNING;

void pce_log_open(int option)
{
	pce_log_stderr = option & LOG_PERROR;
	if (!pce_log_stderr)
		openlog("PCE", option, LOG_USER);
}

void pce_log(int priority, const char *format, ...)
{
	va_list args;

	if (priority > pce_log_priority)
		return;

	va_start(args, format);
	if (pce_log_stderr)
		vfprintf(stderr, format, args);
	else
		vsyslog(priority, format, args);
	va_end(args);
}

void pce_log_nothing(int priority, const char *format, ...)
{
	return;
}

void pce_log_level(int level)
{
	pce_log_priority = level;
}

void pce_log_close(void)
{
	if (!pce_log_stderr)
		closelog();
}
