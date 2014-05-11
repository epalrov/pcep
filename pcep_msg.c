/*
 * pcep_msg.c - PCEP message common header utilities
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#include <stdio.h>
#include <netinet/in.h>
#include <sys/un.h>

#include "pcep_msg.h"

static const char *pcep_msg_type_name[] = {
	"UNKNOWN     ",
	"OPEN        ",
	"KEEPALIVE   ",
	"PC-REQUEST  ",
	"PC-REPLY    ",
	"NOTIFICATION",
	"ERROR       ",
	"CLOSE       ",
	"UNKNOWN     ",
};

#define itos(x) \
	(pcep_msg_type_name[(int)(x) < sizeof(pcep_msg_type_name) ? \
	(int)(x) : (sizeof(pcep_msg_type_name) - 1)])

int pcep_msg_hdr_dump(struct pcep_msg_hdr *msg, char *buf, int count)
{
	return snprintf(buf, count,
		"ver: %d, flags: 0x%02x, type: %s, len: %d",
		(int)(msg->ver), (int)(msg->flags),
		(char *)itos(msg->type), (int)(ntohs(msg->len)));
}

