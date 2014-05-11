/*
 * pcep_session.c - PCEP session
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
#include "pcep_info.h"
#include "pcep_framer.h"

static int pcep_msg_handler(struct pce_session *ses,
	struct pcep_msg_hdr *msg)
{
	int err = 0;

	switch (ses->state) {
	case PCEP_STATE_IDLE:
	case PCEP_STATE_TCP_PENDING:
		/*
		 * no message could be received in those states since the
		 * connection isn't established yet
		 */
		err = -1;
		break;
	case PCEP_STATE_OPEN_WAIT:
		/* check for an open message */
		if (msg->type == PCEP_MSG_TYPE_OPEN &&
			msg->len == 12) {
			/* check session attributes */
			// keepalive frequency =
			// deadtimer =

			/* send a keepalive message */
			/* start keepalive timer */
			/* remote_ok = 1 */

			if (local_ok == 0) {
				ses->state = PCEP_STATE_KEEP_WAIT;
			} else {
				ses->state = PCEP_STATE_SESSION_UP;
			}
		} else {
			/* send a notification */
			// err type = 1 value = 1
			err = -1;
		}
		break;
	case PCEP_STATE_KEEP_WAIT:
		/* check for a keepalive message */
		break;
	case PCEP_STATE_SESSION_UP:
		break;
	default:
		/* invalid state - it should never happen! */
		err = -1;
		break;
	}

	return err;
}

static void pcep_timer_handler(struct pce_session *ses,
	int elaps)
{
	int err = 0;

	switch (ses->state) {
	case PCEP_STATE_IDLE:
		break;
	case PCEP_STATE_TCP_PENDING:
		break;
	case PCEP_STATE_OPEN_WAIT:
		break;
	case PCEP_STATE_KEEP_WAIT:
		break;
	case PCEP_STATE_SESSION_UP:
		break;
	default:
		/* invalid state - it should never happen! */
		err = -1;
		break;
	}

	return err;
}

#define PCEP_FD_SOCKET 0
#define PCEP_FD_TIMER 1
#define PCEP_FD_TOT 2

#define PCEP_MSG_CHUNK 9
void pcep_session_handler(struct pcep_session *ses)
{
	char buf[PCEP_MSG_CHUNK];
	ssize_t count;
	uint64_t elaps;
	struct pcep_msg_hdr *msg;
	struct pollfd fds[PCEP_FD_TOT];

	/* specify events of interest */
	fds[PCEP_FD_SOCKET].fd = ses->cfd;
	fds[PCEP_FD_SOCKET].events = POLLIN;
	fcntl(ses->cfd, F_SETFL, fcntl(ses->cfd, F_GETFL) | O_NONBLOCK);
	fds[PCEP_FD_TIMER].fd = ses->tfd;
	fds[PCEP_FD_TIMER].events = POLLIN;
	fcntl(ses->tfd, F_SETFL, fcntl(ses->tfd, F_GETFL) | O_NONBLOCK);

	while (1) {

		/* block on interested events */
		if (poll(fds, PCEP_FD_TOT, -1) < 0)
			break;

		/* socket data available */
		if (fds[PCEP_FD_SOCKET].revents & POLLIN) {

			/* read a message chunk from the incoming stream */
			do {
				count = read(ses->cfd, buf, PCEP_MSG_CHUNK);
			} while (count == -1 && errno == EINTR);

			/* check for any (fatal) error */
			if (count == -1)
				break;
			/* check if the peer socket hunged-up */
			if (count == 0)
				break;

			/* feed the framer with the message chunk */
			pcep_framer_write(frm, buf, count);

			/* handle PCEP messages (if any) */
			while (msg = pcep_framer_read(frm)) {

				char dump[80];

				pcep_msg_hdr_dump(msg, dump, sizeof(dump));
				pce_log(LOG_DEBUG, "%s\n", dump);

				/* handle the message */
				pcep_msg_handler(ses, msg);

				/* release the message memory */
				pcep_msg_free(msg);
			}
		}

		/* timer expired */
		if (fds[PCEP_FD_TIMER].revents & POLLIN) {

			/* read the expiration time */
			do {
				count = read(ses->tfd, &elaps, sizeof(elaps));
			} while (count == -1 && errno == EINTR);

			/* any error? */
			if (count != sizeof(elaps))
				break;

			pce_log(LOG_DEBUG, "timer exipred, elapsed %d\n", elaps);

			/* handle the timer expiration */
			pcep_timer_handler(ses, elaps);
		}
	}

	return;
}



struct pcep_session_config {
	unsigned int open_wait_timer;
	unsigned int keep_wait_timer;
	unsigned int keep_alive_timer;
	unsigned int dead_timer;
	unsigned int sync_timer;
	unsigned int request_timer;
	unsigned int init_backoff_timer;
	unsigned int max_backoff_timer;
	unsigned int max_req_per_session;
	unsigned int max_unknown_reqs;
	unsigned int max_unknown_msgs
}

struct pcep_session {

	/* session objects */
	int cfd;
	int tfd;
	struct itimerspec tval;
	struct pcep_framer *frm;
	
	/* session status */
	int state;
	int local_id;
	int peer_id;
	
	/* session attributes */
	unsigned int state_last_change;
	unsigned int keep_alive_timer;
	unsigned int peer_keep_alive_timer;
	unsigned int dead_timer;
	unsigned int peer_dead_timer;
	unsigned int keep_alive_hold_time_rem;
	
	/* statistics */
	unsigned int num_pc_req_sent;
	unsigned int num_pc_req_rcvd;
	unsigned int num_pc_rep_sent;
	unsigned int num_pc_rep_rcvd;
	unsigned int num_pc_err_sent;
	unsigned int num_pc_err_rcvd;
	unsigned int num_pc_ntf_sent;
	unsigned int num_pc_ntf_rcvd;
	unsigned int num_keep_alive_sent;
	unsigned int num_keep_alive_rcvd;
	unsigned int num_unknown_rcvd;
}

struct pcep_session *pcep_session_create(int cfd,
	struct pcep_session_config *cfg)
{
	struct pcep_session *ses;

	ses = calloc(1, sizeof(*ses));
	if (!ses) {
		pce_log(LOG_ERR, "failed to get memory\n");
		goto out;
	}

	/* create a PCEP message framer */
	ses->frm = pcep_framer_create();
	if (!ses->frm) {
		pce_log(LOG_ERR, "failed to create PCEP framer\n");
		goto out1;
	}

	/* create a periodic one second timer */
	ses->tfd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (!ses->tfd) {
		pce_log(LOG_ERR, "failed to create PCEP timer\n");
		goto out2;
	}
	ses->tval.it_interval.tv_sec = 1,
	ses->tval.it_value.tv_sec = 1,
	if (timerfd_settime(ses->tfd, 0, &ses->tval, NULL) != 0) {
		pce_log(LOG_ERR, "failed to configure PCEP timer\n");
		goto out3;
	}

	return ses;

out3:
	close(ses->tfd);
out2:
	pcep_framer_delete(ses->frm);
out1:
	free(ses);
out:
	return NULL;
}

void pcep_session_delete(struct pcep_session *ses)
{
	close(ses->tfd);
	pcep_framer_delete(ses->frm);
	free(ses);
}
