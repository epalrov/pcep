/*
 * pcep_obj.c - PCEP message common header utilities
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#include <stdio.h>
#include <netinet/in.h>
#include <sys/un.h>

#include "pcep_obj.h"

static const char *pcep_obj_class_name[] = {
	"UNKNOWN       ",
	"OPEN          ",
	"RP            ",
	"NO-PATH       ",
	"END-POINTS    ",
	"BANDWIDTH     ",
	"METRIC        ",
	"ERO           ",
	"RRO           ",
	"LSPA          ",
	"IRO           ",
	"SVEC          ",
	"NOTIFICATION  ",
	"PCEP-ERROR    ",
	"LOAD-BALANCING",
	"CLOSE         ",
	"UNKNOWN       ",
};

#define itos(x) \
	(pcep_obj_class_name[(int)(x) < sizeof(pcep_obj_class_name) ? \
	(int)(x) : (sizeof(pcep_obj_class_name) - 1)])

int pcep_obj_hdr_dump(struct pcep_obj_hdr *obj, char *buf, int count)
{
	return snprintf(buf, count,
		"class: %s, type: %d, P: %d, I: %d, len: %d",
		(char *)itos(obj->o_class), (int)(obj->o_type),
		(int)(obj->p_flag), (int)(obj->i_flag),
		(int)(ntohs(obj->len)));
}

