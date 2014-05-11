/*
 * pcep_msg.h - PCEP message common header interface
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#ifndef PCEP_MSG_H
#define PCEP_MSG_H

/*
 *  PCEP Message Common Header
 *
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Ver |  Flags  |  Message-Type |       Message-Length          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 *  Ver (Version - 3 bits):  PCEP version number.  Current version is
 *     version 1.
 *
 *  Flags (5 bits):  No flags are currently defined.  Unassigned bits are
 *     considered as reserved.  They MUST be set to zero on transmission
 *     and MUST be ignored on receipt.
 *
 *  Message-Type (8 bits):  The following message types are currently
 *     defined:
 *
 *        Value    Meaning
 *          1        Open
 *          2        Keepalive
 *          3        Path Computation Request
 *          4        Path Computation Reply
 *          5        Notification
 *          6        Error
 *          7        Close
 *
 *  Message-Length (16 bits):  total length of the PCEP message including
 *     the common header, expressed in bytes.
 */

#define PCEP_MSG_VERSION              1

#define PCEP_MSG_TYPE_MIN             0
#define PCEP_MSG_TYPE_OPEN            1
#define PCEP_MSG_TYPE_KEEPALIVE       2
#define PCEP_MSG_TYPE_PC_REQUEST      3
#define PCEP_MSG_TYPE_PC_REPLY        4
#define PCEP_MSG_TYPE_NOTIFICATION    5
#define PCEP_MSG_TYPE_ERROR           6
#define PCEP_MSG_TYPE_CLOSE           7
#define PCEP_MSG_TYPE_MAX             8

struct pcep_msg_hdr {
	unsigned char ver : 3;
	unsigned char flags : 5;
	unsigned char type;
	unsigned short len;
} __attribute__ ((packed));

#define PCEP_MSG_HDR_SIZE (sizeof(struct pcep_msg_hdr))

extern int pcep_msg_hdr_dump(struct pcep_msg_hdr *msg, char *buf, int count);

#endif /* PCEP_MSG_H */
