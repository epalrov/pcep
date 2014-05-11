/*
 * pcep_info.h - PCEP information model (based on PCEP MIB)
 *
 * Copyright (C) 2013 Paolo Rovelli
 *
 * Author: Paolo Rovelli <paolorovelli@yahoo.it>
 */

#ifndef PCEP_INFO_H
#define PCEP_INFO_H

/*
 * PcePcepEntityEntry ::=
 * SEQUENCE {
 *         pcePcepEntityIndex               Integer32,         (NA)
 *         pcePcepEntityRowStatus           RowStatus,         (RW)
 *         pcePcepEntityAdminStatus         INTEGER,           (RW)
 *         pcePcepEntityOperStatus          INTEGER,           (RO)
 *         pcePcepEntityAddrType            InetAddressType,   (RW)
 *         pcePcepEntityAddr                InetAddress,       (RW)
 *         pcePcepEntityTcpPort             InetPortNumber,    (RW)
 *         pcePcepEntityConnectTimer        Unsigned32,        (RW)
 *         pcePcepEntityOpenWaitTimer       Unsigned32,        (RW)
 *         pcePcepEntityKeepWaitTimer       Unsigned32,        (RW)
 *         pcePcepEntityKeepAliveTimer      Unsigned32,        (RW)
 *         pcePcepEntityDeadTimer           Unsigned32,        (RW)
 *         pcePcepEntitySyncTimer           Unsigned32,        (RW)
 *         pcePcepEntityRequestTimer        Unsigned32,        (RW)
 *         pcePcepEntityInitBackoffTimer    Unsigned32,        (RW)
 *         pcePcepEntityMaxBackoffTimer     Unsigned32,        (RW)
 *         pcePcepEntityMaxSessions         Unsigned32,        (RW)
 *         pcePcepEntityMaxReqPerSession    Unsigned32,        (RW)
 *         pcePcepEntityMaxUnknownReqs      Unsigned32,        (RW)
 *         pcePcepEntityMaxUnknownMsgs      Unsigned32         (RW)
 * }
 * INDEX {
 *         pcePcepEntityIndex
 * }
 */

struct pcep_entity_info {
	int row_status;
	int admin_status;
	int oper_status;
	unsigned int addr_type;
	unsigned int addr;
	unsigned short tcp_port;
	unsigned int connect_timer;
	unsigned int open_wait_timer;
	unsigned int keep_wait_timer;
	unsigned int keep_alive_timer;
	unsigned int dead_timer;
	unsigned int sync_timer;
	unsigned int request_timer;
	unsigned int init_backoff_timer;
	unsigned int max_backoff_timer;
	unsigned int max_sessions;
	unsigned int max_req_per_session;
	unsigned int max_unknown_reqs;
	unsigned int max_unknown_msgs;
};

#define PCEP_ADMIN_STATUS_UP         (1) /* active                             */
#define PCEP_ADMIN_STATUS_DOWN       (2) /* inactive                           */

#define PCEP_OPER_STATUS_UP          (1) /* active                             */
#define PCEP_OPER_STATUS_DOWN        (2) /* inactive                           */
#define PCEP_OPER_STATUS_GOING_UP    (3) /* activating                         */
#define PCEP_OPER_STATUS_GOING_DOWN  (4) /* deactivating                       */
#define PCEP_OPER_STATUS_FAILED      (5) /* failed, will recover when possible */
#define PCEP_OPER_STATUS_FAILED_PERM (6) /* operator intervention              */

/*
 * PcePcepPeerEntry ::=
 * SEQUENCE {
 *         pcePcepPeerAddrType              InetAddressType,   (NA)
 *         pcePcepPeerAddr                  InetAddress,       (NA)
 *         pcePcepPeerSessionExists         TruthValue,        (RO)
 *         pcePcepPeerNumSessSetupOK        Counter32,         (RO)
 *         pcePcepPeerNumSessSetupFail      Counter32,         (RO)
 *         pcePcepPeerSessionUpTime         TimeStamp,         (RO)
 *         pcePcepPeerSessionFailTime       TimeStamp,         (RO)
 *         pcePcepPeerResponseTime          Unsigned32         (RO)
 * }
 * INDEX {
 *         pcePcepEntityIndex,
 *         pcePcepPeerAddrType,
 *         pcePcepPeerAddr
 * }
 */
struct pcep_peer_info {
	unsigned int addr_type;
	unsigned int addr;
	int session_exists;
	unsigned int num_sess_setup_ok;
	unsigned int num_sess_setup_fail;
	unsigned int session_up_time;
	unsigned int session_fail_time;
	unsigned int response_time;
};

/* PcePcepSessionEntry ::=
 * SEQUENCE {
 *         pcePcepSessionStateLastChange    TimeStamp,         (RO)
 *         pcePcepSessionState              INTEGER,           (RO)
 *         pcePcepSessionLocalID            Integer32,         (RO)
 *         pcePcepSessionPeerID             Integer32,         (RO)
 *         pcePcepSessionKeepaliveTimer     Unsigned32,        (RO)
 *         pcePcepSessionPeerKeepaliveTimer Unsigned32,        (RO)
 *         pcePcepSessionDeadTimer          Unsigned32,        (RO)
 *         pcePcepSessionPeerDeadTimer      Unsigned32,        (RO)
 *         pcePcepSessionKAHoldTimeRem      TimeInterval,      (RO)
 *         pcePcepSessionNumPCReqSent       Counter32,         (RO)
 *         pcePcepSessionNumPCReqRcvd       Counter32,         (RO)
 *         pcePcepSessionNumPCRepSent       Counter32,         (RO)
 *         pcePcepSessionNumPCRepRcvd       Counter32,         (RO)
 *         pcePcepSessionNumPCErrSent       Counter32,         (RO)
 *         pcePcepSessionNumPCErrRcvd       Counter32,         (RO)
 *         pcePcepSessionNumPCNtfSent       Counter32,         (RO)
 *         pcePcepSessionNumPCNtfRcvd       Counter32,         (RO)
 *         pcePcepSessionNumKeepaliveSent   Counter32,         (RO)
 *         pcePcepSessionNumKeepaliveRcvd   Counter32,         (RO)
 *         pcePcepSessionNumUnknownRcvd     Counter32          (RO)
 * }
 * INDEX {
 *         pcePcepEntityIndex,
 *         pcePcepPeerAddrType,
 *         pcePcepPeerAddr
 * }
 */
struct pcep_session_info {
	unsigned int state_last_change;
	int state;
	int local_id;
	int peer_id;
	unsigned int keep_alive_timer;
	unsigned int peer_keep_alive_timer;
	unsigned int dead_timer;
	unsigned int peer_dead_timer;
	unsigned int keep_alive_hold_time_rem;
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

#define PCEP_STATE_IDLE        (0)
#define PCEP_STATE_TCP_PENDING (1)
#define PCEP_STATE_OPEN_WAIT   (2)
#define PCEP_STATE_KEEP_WAIT   (3)
#define PCEP_STATE_SESSION_UP  (4)

#endif /* PCEP_INFO_H */
