/** Copyright (c) 1993, 1996  Mentat Inc.
 ** tihdr.h 43.2, last change 10/03/01
 **/

#ifndef _TIHDR_
#define _TIHDR_

#ifdef MI_H_ID_STRINGS
static char	tihdr_h_sccsid[] = "@(#)tihdr.h	43.2";
#endif

/* User generated requests */
#define	T_BIND_REQ		1
#define	T_CONN_REQ		2	/* connection request */
#define	T_CONN_RES		3	/* respond to connection indication */
#define	T_DATA_REQ		4
#define	T_DISCON_REQ		5
#define	T_EXDATA_REQ		6
#define	T_INFO_REQ		7
#define	T_OPTMGMT_REQ		8
#define	T_ORDREL_REQ		9
#define	T_UNBIND_REQ		10
#define	T_UNITDATA_REQ		11
#define	T_ADDR_REQ		12	/* TPI 1.5 */

/* Transport generated indications and acknowledgements */
#define	T_BIND_ACK		20
#define	T_CONN_CON		21	/* connection confirmation */
#define	T_CONN_IND		22	/* incoming connection indication */
#define	T_DATA_IND		23
#define	T_DISCON_IND		24
#define	T_ERROR_ACK		25
#define	T_EXDATA_IND		26
#define	T_INFO_ACK		27
#define	T_OK_ACK		28
#define	T_OPTMGMT_ACK		29
#define	T_ORDREL_IND		30
#define	T_UNITDATA_IND		31
#define	T_UDERROR_IND		32
#define T_ADDR_ACK		33	/* TPI 1.5 */
#define	T_OPTDATA_IND		34
#define	T_OPTDATA_REQ		35

/* State values */
#define	TS_UNBND		1
#define	TS_WACK_BREQ		2
#define	TS_WACK_UREQ		3
#define	TS_IDLE			4
#define	TS_WACK_OPTREQ		5
#define	TS_WACK_CREQ		6
#define	TS_WCON_CREQ		7
#define	TS_WRES_CIND		8
#define	TS_WACK_CRES		9
#define	TS_DATA_XFER		10
#define	TS_WIND_ORDREL		11
#define	TS_WREQ_ORDREL		12
#define	TS_WACK_DREQ6		13
#define	TS_WACK_DREQ7		14
#define	TS_WACK_DREQ9		15
#define	TS_WACK_DREQ10		16
#define	TS_WACK_DREQ11		17
#define	TS_WACK_ORDREL		18
#define	TS_NOSTATES		19
#define	TS_BAD_STATE		19

/* Transport events */
#define	TE_OPENED		1
#define	TE_BIND			2
#define	TE_OPTMGMT		3
#define	TE_UNBIND		4
#define	TE_CLOSED		5
#define	TE_CONNECT1		6
#define	TE_CONNECT2		7
#define	TE_ACCEPT1		8
#define	TE_ACCEPT2		9
#define	TE_ACCEPT3		10
#define	TE_SND			11
#define	TE_SNDDIS1		12
#define	TE_SNDDIS2		13
#define	TE_SNDREL		14
#define	TE_SNDUDATA		15
#define	TE_LISTEN		16
#define	TE_RCVCONNECT		17
#define	TE_RCV			18
#define	TE_RCVDIS1		19
#define	TE_RCVDIS2		20
#define	TE_RCVDIS3		21
#define	TE_RCVREL		22
#define	TE_RCVUDATA		23
#define	TE_RCVUDERR		24
#define	TE_PASS_CONN		25
#define	TE_BAD_EVENT		26

struct T_addr_req {
	long    PRIM_type;      /* always T_ADDR_REQ */
};
	        
struct T_addr_ack {
	long    PRIM_type;      /* always T_ADDR_ACK */
	long    LOCADDR_length; /* length of local address */
	long    LOCADDR_offset; /* offset of local address */
	long    REMADDR_length; /* length of remote address */
	long    REMADDR_offset; /* offset of remote address */
};

struct T_bind_ack {
	long	PRIM_type;	/* always T_BIND_ACK */
	long	ADDR_length;
	long	ADDR_offset;
	unsigned long	CONIND_number;
};

struct T_bind_req {
	long	PRIM_type;	/* always T_BIND_REQ */
	long	ADDR_length;
	long	ADDR_offset;
	unsigned long	CONIND_number;
};

struct T_conn_con {
	long	PRIM_type;	/* always T_CONN_CON */
	long	RES_length;	/* responding address length */
	long	RES_offset;
	long	OPT_length;
	long	OPT_offset;
};

struct T_conn_ind {
	long	PRIM_type;	/* always T_CONN_IND */
	long	SRC_length;
	long	SRC_offset;
	long	OPT_length;
	long	OPT_offset;
	long	SEQ_number;
};

struct T_conn_req {
	long	PRIM_type;	/* always T_CONN_REQ */
	long	DEST_length;
	long	DEST_offset;
	long	OPT_length;
	long	OPT_offset;
};

struct T_conn_res {
	long	PRIM_type;	/* always T_CONN_RES */
	struct queue * QUEUE_ptr;
	long	OPT_length;
	long	OPT_offset;
	long	SEQ_number;
};

struct T_data_ind {
	long	PRIM_type;	/* always T_DATA_IND */
	long	MORE_flag;
};

struct T_data_req {
	long	PRIM_type;	/* always T_DATA_REQ */
	long	MORE_flag;
};

struct T_discon_ind {
	long	PRIM_type;	/* always T_DISCON_IND */
	long	DISCON_reason;
	long	SEQ_number;
};

struct T_discon_req {
	long	PRIM_type;	/* always T_DISCON_REQ */
	long	SEQ_number;
};

struct T_exdata_ind {
	long	PRIM_type;	/* always T_EXDATA_IND */
	long	MORE_flag;
};

struct T_exdata_req {
	long	PRIM_type;	/* always T_EXDATA_REQ */
	long	MORE_flag;
};

struct T_error_ack {
	long	PRIM_type;	/* always T_ERROR_ACK */
	long	ERROR_prim;	/* primitive in error */
	long	TLI_error;
	long	UNIX_error;
};

struct T_info_ack {
	long	PRIM_type;	/* always T_INFO_ACK */
	long	TSDU_size;	/* max TSDU size */
	long	ETSDU_size;	/* max ETSDU size */
	long	CDATA_size;	/* connect data size */
	long	DDATA_size;	/* disconnect data size */
	long	ADDR_size;	/* TSAP size */
	long	OPT_size;	/* options size */
	long	TIDU_size;	/* TIDU size */
	long	SERV_type;	/* service type */
	long	CURRENT_state;	/* current state */
	long    PROVIDER_flag;  /* provider flags (see xti.h for defines) */
};

/* Provider flags */
#define SENDZERO        0x001   /* supports 0-length TSDU's */
#define XPG4_1          0x002   /* provider supports recent stuff */

struct T_info_req {
	long	PRIM_type;	/* always T_INFO_REQ */
};

struct T_ok_ack {
	long	PRIM_type;	/* always T_OK_ACK */
	long	CORRECT_prim;
};

struct T_optdata_ind {
	long	PRIM_type;	/* always T_OPTDATA_IND */
	long	DATA_flag;	/* flag bits associated with data */
	long	OPT_length;	/* options length */
	long	OPT_offset;	/* options offset */
};

struct T_optdata_req {
	long	PRIM_type;	/* always T_OPTDATA_REQ */
	long	DATA_flag;	/* flag bits associated with data */
	long	OPT_length;	/* options length */
	long	OPT_offset;	/* options offset */
};

struct T_optmgmt_ack {
	long	PRIM_type;	/* always T_OPTMGMT_ACK */
	long	OPT_length;
	long	OPT_offset;
	long	MGMT_flags;
};

struct T_optmgmt_req {
	long	PRIM_type;	/* always T_OPTMGMT_REQ */
	long	OPT_length;
	long	OPT_offset;
	long	MGMT_flags;
};

struct T_ordrel_ind {
	long	PRIM_type;	/* always T_ORDREL_IND */
};

struct T_ordrel_req {
	long	PRIM_type;	/* always T_ORDREL_REQ */
};

struct T_unbind_req {
	long	PRIM_type;	/* always T_UNBIND_REQ */
};

struct T_uderror_ind {
	long	PRIM_type;	/* always T_UDERROR_IND */
	long	DEST_length;
	long	DEST_offset;
	long	OPT_length;
	long	OPT_offset;
	long	ERROR_type;
};

struct T_unitdata_ind {
	long	PRIM_type;	/* always T_UNITDATA_IND */
	long	SRC_length;
	long	SRC_offset;
	long	OPT_length;
	long	OPT_offset;
};

struct T_unitdata_req {
	long	PRIM_type;	/* always T_UNITDATA_REQ */
	long	DEST_length;
	long	DEST_offset;
	long	OPT_length;
	long	OPT_offset;
};

union T_primitives {
	long			type;
	struct T_addr_req	taddrreq;
	struct T_addr_ack	taddrack;
	struct T_bind_ack	tbindack;
	struct T_bind_req	tbindreq;
	struct T_conn_con	tconncon;
	struct T_conn_ind	tconnind;
	struct T_conn_req	tconnreq;
	struct T_conn_res	tconnres;
	struct T_data_ind	tdataind;
	struct T_data_req	tdatareq;
	struct T_discon_ind	tdisconind;
	struct T_discon_req	tdisconreq;
	struct T_exdata_ind	texdataind;
	struct T_exdata_req	texdatareq;
	struct T_error_ack	terrorack;
	struct T_info_ack	tinfoack;
	struct T_info_req	tinforeq;
	struct T_ok_ack		tokack;
	struct T_optdata_ind	toptdataind;
	struct T_optdata_req	toptdatareq;
	struct T_optmgmt_ack	toptmgmtack;
	struct T_optmgmt_req	toptmgmtreq;
	struct T_ordrel_ind	tordrelind;
	struct T_ordrel_req	tordrelreq;
	struct T_unbind_req	tunbindreq;
	struct T_uderror_ind	tuderrorind;
	struct T_unitdata_ind	tunitdataind;
	struct T_unitdata_req	tunitdatareq;
};

#endif	/* _TIHDR_ */
