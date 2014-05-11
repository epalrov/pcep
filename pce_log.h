/*
 * syslog.h - PCE logger interface
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#ifndef PCE_LOG_H
#define PCE_LOG_H

#include <stdarg.h>
#include <syslog.h>

extern void pce_log_open(int option);
extern void pce_log_level(int level);
extern void pce_log(int priority, const char *format, ...);
extern void pce_log_nothing(int priority, const char *format, ...);
extern void pce_log_close(void);

#define pce_log_emerg(format, ...) \
	pce_log(LOG_EMERG, format, ##__VA_ARGS__)
#define pce_log_alert(format, ...) \
	pce_log(LOG_ALERT, format, ##__VA_ARGS__)
#define pce_log_crit(format, ...) \
	pce_log(LOG_CRIT, format, ##__VA_ARGS__)
#define pce_log_err(format, ...) \
	pce_log(LOG_ERR, format, ##__VA_ARGS__)
#define pce_log_warning(format, ...) \
	pce_log(LOG_WARNING, format, ##__VA_ARGS__)
#define pce_log_warn pce_log_warning
#define pce_log_notice(format, ...) \
	pce_log(LOG_NOTICE, format, ##__VA_ARGS__)
#define pce_log_info(format, ...) \
	pce_log(LOG_INFO, format, ##__VA_ARGS__)

/* pce_log_debug() should produce zero code unless DEBUG is defined */
#ifdef DEBUG
#define pce_log_debug(format, ...) \
	pce_log(LOG_DEBUG, format, ##__VA_ARGS__)
#else
#define pce_log_debug(format, ...) \
	pce_log_nothing(LOG_DEBUG, format, ##__VA_ARGS__)
#endif

#endif /* PCE_LOG_H */
