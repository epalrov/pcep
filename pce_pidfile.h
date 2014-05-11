/*
 * pce_pidfile.h - PCE pidfile interface
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolor.rovelli@ericsson.com>
 */

#ifndef PCE_PIDEFILE_H
#define PCE_PIDEFILE_H

extern int pce_pidfile_create(char *pathname);
extern int pce_pidfile_delete(char *pathname);
extern int pce_pidfile_check(char *pathname);

#endif /*  PCE_PIDEFILE_H */
