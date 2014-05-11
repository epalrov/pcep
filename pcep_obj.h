/*
 * pcep_obj.h - PCEP common object interface
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#ifndef PCEP_OBJ_H
#define PCEP_OBJ_H

/*
 *  PCEP Common Object Header
 *
 *  A PCEP object carried within a PCEP message consists of one or more
 *  32-bit words with a common header that has the following format:
 *
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Object-Class  |   OT  |Res|P|I|   Object Length (bytes)       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  //                        (Object body)                        //
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 *  Object-Class (8 bits):  identifies the PCEP object class.
 *
 *  OT (Object-Type - 4 bits):  identifies the PCEP object type.
 *
 *     The Object-Class and Object-Type fields are managed by IANA.
 *
 *     The Object-Class and Object-Type fields uniquely identify each
 *     PCEP object.
 *
 *  Res flags (2 bits):  Reserved field.  This field MUST be set to zero
 *     on transmission and MUST be ignored on receipt.
 *
 *  P flag (Processing-Rule - 1-bit):  the P flag allows a PCC to specify
 *     in a PCReq message sent to a PCE whether the object must be taken
 *     into account by the PCE during path computation or is just
 *     optional.  When the P flag is set, the object MUST be taken into
 *     account by the PCE.  Conversely, when the P flag is cleared, the
 *     object is optional and the PCE is free to ignore it.
 *
 *  I flag (Ignore - 1 bit):  the I flag is used by a PCE in a PCRep
 *     message to indicate to a PCC whether or not an optional object was
 *     processed.  The PCE MAY include the ignored optional object in its
 *     reply and set the I flag to indicate that the optional object was
 *     ignored during path computation.  When the I flag is cleared, the
 *     PCE indicates that the optional object was processed during the
 *     path computation.  The setting of the I flag for optional objects
 *     is purely indicative and optional.  The I flag has no meaning in a
 *     PCRep message when the P flag has been set in the corresponding
 *     PCReq message.
 */

#define PCEP_OBJ_CLASS_MIN             0
#define PCEP_OBJ_CLASS_MAX            16

#define PCEP_OBJ_CLASS_OPEN            1
#define PCEP_OBJ_CLASS_RP              2
#define PCEP_OBJ_CLASS_NO_PATH         3
#define PCEP_OBJ_CLASS_END_POINTS      4
#define PCEP_OBJ_CLASS_BANDWIDTH       5
#define PCEP_OBJ_CLASS_METRIC          6
#define PCEP_OBJ_CLASS_ERO             7
#define PCEP_OBJ_CLASS_RRO             8
#define PCEP_OBJ_CLASS_LSPA            9
#define PCEP_OBJ_CLASS_IRO            10
#define PCEP_OBJ_CLASS_SVEC           11
#define PCEP_OBJ_CLASS_NOTIFICATION   12
#define PCEP_OBJ_CLASS_PCEP_ERROR     13
#define PCEP_OBJ_CLASS_LOAD_BALANCING 14
#define PCEP_OBJ_CLASS_CLOSE          15

struct pcep_obj_hdr {
	unsigned char o_class;
	unsigned char o_type : 4;
	unsigned char res : 2;
	unsigned char p_flag : 1;
	unsigned char i_flag : 1;
	unsigned short len;
} __attribute__ ((packed));

#define PCEP_OBJ_HDR_SIZE (sizeof(struct pcep_obj_hdr))

extern int pcep_obj_hdr_dump(struct pcep_obj_hdr *obj, char *str, int count);

#endif /* PCEP_OBJ_H */
