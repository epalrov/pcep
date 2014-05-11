/*
 * pcep_framer.c - PCEP framer implementation
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include "pcep_framer.h"

/*
 * pcep_framer_create - Create a new instance of PCEP framer
 */
struct pcep_framer *pcep_framer_create(void)
{
	struct pcep_framer *f;

	f = calloc(1, sizeof(struct pcep_framer));
	f->state = PCEP_HUNT_MSG_VER_FLAGS;
	INIT_LIST_HEAD(&f->msg_list.list);

	return f;
}

/*
 * pcep_framer_delete - Delete the specified instance of PCEP framer
 */
void pcep_framer_delete(struct pcep_framer *f)
{
	struct pcep_msg_list *msg_entry;

	/* release any message in the message queue */
	while (!list_empty(&f->msg_list.list)) {
		msg_entry = list_entry(f->msg_list.list.next,
			struct pcep_msg_list, list);
		free(msg_entry->msg);
		list_del(&msg_entry->list);
		free(msg_entry);
	}

	/* release the buffer for the message recording */
	if (f->msg_curr)
		free(f->msg_curr);

	/* release the framer */
	free(f);
}

/*
 * pcep_framer_reset - Reset the FSM of the PCEP framer
 */
void pcep_framer_reset(struct pcep_framer *f)
{
	f->state = PCEP_HUNT_MSG_VER_FLAGS;
	if (f->msg_curr) {
		free(f->msg_curr);
		f->msg_curr = NULL;
	}
}

#define PCEP_MSG_VER_MASK 0xE0
#define PCEP_MSG_VER_SHIFT 5
#define PCEP_MSG_FLAGS_MASK 0x1F
#define PCEP_MSG_FLAGS_SHIFT 0

/*
 * pcep_framer_write - Feed the FSM of the PCEP framer with a message chunk
 */
void pcep_framer_write(struct pcep_framer *f, char *buf, size_t size)
{
	int i;

	for (i = 0; i < size; i++) {
		switch (f->state) {
		case PCEP_HUNT_MSG_VER_FLAGS:
			f->hdr_curr.ver = (buf[i] & PCEP_MSG_VER_MASK)
				>> PCEP_MSG_VER_SHIFT;
			f->hdr_curr.flags = (buf[i] & PCEP_MSG_FLAGS_MASK)
				 >> PCEP_MSG_FLAGS_SHIFT;
			if (f->hdr_curr.ver == PCEP_MSG_VERSION) {
				f->state = PCEP_HUNT_MSG_TYPE;
			} else {
				pcep_framer_reset(f);
			}
			break;

		case PCEP_HUNT_MSG_TYPE:
			f->hdr_curr.type = buf[i];
			if (f->hdr_curr.type > PCEP_MSG_TYPE_MIN &&
				f->hdr_curr.type < PCEP_MSG_TYPE_MAX) {
				f->msg_len = 0;
				f->msg_len_cnt = 0;
				f->state = PCEP_HUNT_MSG_LEN;
			} else {
				pcep_framer_reset(f);
			}
			break;

		case PCEP_HUNT_MSG_LEN:
			/* the MSB of the length field is received first */
			memcpy((char *)(&f->hdr_curr.len) + f->msg_len_cnt,
				&buf[i], 1);
			f->msg_len_cnt++;
			if (f->msg_len_cnt == sizeof(f->hdr_curr.len)) {
				/* allocate a buffer to store the message */
				f->msg_len = ntohs(f->hdr_curr.len);
				if (f->msg_len >= PCEP_MSG_HDR_SIZE) {
					if (!f->msg_curr) {
						f->msg_curr = malloc(f->msg_len);
						if (!f->msg_curr) {
							pcep_framer_reset(f);
							continue;
						}
						memcpy(f->msg_curr, &f->hdr_curr,
							PCEP_MSG_HDR_SIZE);
						f->msg_pos = PCEP_MSG_HDR_SIZE;
					}
					f->state = PCEP_HUNT_MSG;
				} else {
					pcep_framer_reset(f);
				}
			}
			/*
			 * if the message has no body (e.g. keepalive),
			 * continue to process the message in PCEP_HUNT_MSG
			 */
			if (f->msg_len != PCEP_MSG_HDR_SIZE)
				break;

		case PCEP_HUNT_MSG:
			/* message body recording */
			if (f->msg_pos < f->msg_len) {
				((char *)(f->msg_curr))[f->msg_pos] = buf[i];
				f->msg_pos++;
			}

			/* add the recorded message to the message queue */
			if (f->msg_pos == f->msg_len) {
				struct pcep_msg_list *msg_entry;

				msg_entry = malloc(sizeof(*msg_entry));
				if (!msg_entry) {
					pcep_framer_reset(f);
					continue;
				}
				msg_entry->msg = f->msg_curr;
				list_add_tail(&msg_entry->list,
					&f->msg_list.list);
				f->msg_curr = NULL;
				f->state = PCEP_HUNT_MSG_VER_FLAGS;
			}
			break;

		default:
			/* should never happen! */
			pcep_framer_reset(f);
		}
	}
}

/*
 * pcep_framer_read - Return a PCEP message (if any) from the PCEP farmer queue
 */
extern struct pcep_msg_hdr *pcep_framer_read(struct pcep_framer *f)
{
	struct pcep_msg_hdr *m = NULL;
	struct pcep_msg_list *msg_entry;

	/* remove a message from the message queue */
	if (!list_empty(&f->msg_list.list)) {
		msg_entry = list_entry(f->msg_list.list.next,
			struct pcep_msg_list, list);
		m = msg_entry->msg;
		list_del(&msg_entry->list);
		free(msg_entry);
	}

	return m;
}

/*
 * pcep_msg_free - Release a PCEP message
 */
extern void pcep_msg_free(struct pcep_msg_hdr *m)
{
	if (!m)
		free(m);
}
