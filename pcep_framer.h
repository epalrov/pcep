/*
 * pcep_framer.h - PCEP framer interface
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#ifndef PCEP_FRAMER_H
#define PCEP_FRAMER_H

#include "list.h"
#include "pcep_msg.h"

struct pcep_msg_list {
	struct list_head list;
	struct pcep_msg_hdr *msg;
};

enum pcep_hunt_state {
	PCEP_HUNT_MSG_VER_FLAGS = 0,
	PCEP_HUNT_MSG_TYPE,
	PCEP_HUNT_MSG_LEN,
	PCEP_HUNT_MSG
};

struct pcep_framer {
	/* current FSM state */
	enum pcep_hunt_state state;

	/* current recording message */
	struct pcep_msg_hdr hdr_curr;
	struct pcep_msg_hdr *msg_curr;
	unsigned int msg_pos;
	unsigned short msg_len;
	unsigned short msg_len_cnt;

	/* received message list */
	struct pcep_msg_list msg_list;
};

extern struct pcep_framer *pcep_framer_create(void);
extern void pcep_framer_delete(struct pcep_framer *f);
extern void pcep_framer_reset(struct pcep_framer *f);

extern void pcep_framer_write(struct pcep_framer *f, char *buf, size_t size);
extern struct pcep_msg_hdr *pcep_framer_read(struct pcep_framer *f);
extern void pcep_msg_free(struct pcep_msg_hdr *m);

#endif /* PCEP_FRAMER_H */
