/** Copyright (c) 1989-2002  Mentat Inc.
 ** arp.h 43.4, last change 09/19/02
 **/

#ifndef _ARP_
#define _ARP_

#ifdef MI_H_ID_STRINGS
static char	arp_h_sccsid[] = "@(#)arp.h	43.4";
#endif

#include <streams/types.h>
#include <streams/stream.h>
/*
 * These commands and structures may be defined in another header file.
 * If so ARP_EXTERNAL_DEFINITIONS will be defined.
 */
#ifndef	ARP_EXTERNAL_DEFINITIONS

/**
 ** ARP Command Codes - arc_cmd field of arc_t
 **/
#define	AR_ENTRY_ADD		MIOC_CMD(MIOC_ARP,1)
#define	AR_ENTRY_DELETE		MIOC_CMD(MIOC_ARP,2)
#define	AR_ENTRY_QUERY		MIOC_CMD(MIOC_ARP,3)
#define	AR_XMIT_REQUEST		MIOC_CMD(MIOC_ARP,4)
#define	AR_XMIT_TEMPLATE	MIOC_CMD(MIOC_ARP,5)
#define	AR_ENTRY_SQUERY		MIOC_CMD(MIOC_ARP,6)
#define	AR_MAPPING_ADD		MIOC_CMD(MIOC_ARP,7)
#define	AR_CLIENT_NOTIFY	MIOC_CMD(MIOC_ARP,8)
#define	AR_INTERFACE_UP		MIOC_CMD(MIOC_ARP,9)
#define	AR_INTERFACE_DOWN	MIOC_CMD(MIOC_ARP,10)
#define	AR_INTERFACE_INDEX	MIOC_CMD(MIOC_ARP,11)
#define	AR_XMIT_RESPONSE	MIOC_CMD(MIOC_ARP,12)

/* AR_INTERFACE_UP - arc_flags */
#define	ARC_UP_F_SNAP		0x1	/* Perform for SNAP encapsulation */
#define	ARC_UP_F_PLAINSAP	0x2	/* Perform straight SAP encapsulation */

/* ace_flags */
#define	ACE_F_PERMANENT		0x1
#define	ACE_F_PUBLISH		0x2
#define	ACE_F_DYING		0x4
#define	ACE_F_RESOLVED		0x8
#define ACE_F_MAPPING		0x10	/** Use bit mask on target address */
#define	ACE_F_LOCAL		0x20
#define	ACE_F_TENTATIVE		0x40	/* In address probing phase */
#define	ACE_F_ANNOUNCE		0x80	/* In address announcement phase */

/** 
 ** ARP Command Structures 
 **/
typedef struct ar_cmd_s {		/** ARP Command Structure */
	uint	arc_cmd;
	uint	arc_name_offset;
	uint	arc_name_length;
	uint	arc_proto;
	uint	arc_proto_addr_offset;
	uint	arc_proto_addr_length;
	uint	arc_flags;
	union {
		uint	arc_u_index;
		void	* arc_u_reserved;
	} arc_u;
} arc_t;
#define	arc_index	arc_u.arc_u_index

typedef	struct ar_entry_add_s {
	arc_t	area_arc;
	uint	area_proto_mask_offset;
	uint	area_hw_addr_offset;
	uint	area_hw_addr_length;
} area_t;
#define	area_cmd		area_arc.arc_cmd
#define	area_name_offset	area_arc.arc_name_offset
#define	area_name_length	area_arc.arc_name_length
#define	area_proto		area_arc.arc_proto
#define	area_proto_addr_offset	area_arc.arc_proto_addr_offset
#define	area_proto_addr_length	area_arc.arc_proto_addr_length
#define	area_flags		area_arc.arc_flags

typedef struct ar_entry_delete_s {
	arc_t	ared_arc;
	uint	ared_proto_mask_offset;
} ared_t;
#define	ared_cmd		ared_arc.arc_cmd
#define	ared_name_offset	ared_arc.arc_name_offset
#define	ared_name_length	ared_arc.arc_name_length
#define	ared_proto		ared_arc.arc_proto
#define	ared_proto_addr_offset	ared_arc.arc_proto_addr_offset
#define	ared_proto_addr_length	ared_arc.arc_proto_addr_length
#define	ared_flags		ared_arc.arc_flags

typedef	struct ar_entry_query_s {
	arc_t	areq_arc;
	uint	areq_sender_addr_offset;
	uint	areq_sender_addr_length;
	uint	areq_xmit_count;	/* 0 ==> cache lookup only */
	uint	areq_xmit_interval;	/* # of milliseconds; 0: default */
	uint	areq_max_buffered;	/* # ofquests to buffer; 0: default */
	uchar	areq_sap[8];		/* to insert in returned template */
} areq_t;
#define	areq_cmd		areq_arc.arc_cmd
#define	areq_name_offset	areq_arc.arc_name_offset
#define	areq_name_length	areq_arc.arc_name_length
#define	areq_proto		areq_arc.arc_proto
#define	areq_target_addr_offset	areq_arc.arc_proto_addr_offset
#define	areq_target_addr_length	areq_arc.arc_proto_addr_length
#define	areq_flags		areq_arc.arc_flags

typedef	struct ar_mapping_add_s {
	arc_t	arma_arc;
	uint	arma_proto_mask_offset;
	uint	arma_proto_extract_mask_offset;
	uint	arma_hw_addr_offset;
	uint	arma_hw_addr_length;
	uint	arma_hw_mapping_start;		/* Offset to start of
						 * the mask&proto_addr */
} arma_t;
#define	arma_cmd		arma_arc.arc_cmd
#define	arma_name_offset	arma_arc.arc_name_offset
#define	arma_name_length	arma_arc.arc_name_length
#define	arma_proto		arma_arc.arc_proto
#define	arma_proto_addr_offset	arma_arc.arc_proto_addr_offset
#define	arma_proto_addr_length	arma_arc.arc_proto_addr_length
#define	arma_flags		arma_arc.arc_flags

/** Structure used to notify clients of interesting conditions. */
typedef struct ar_client_notify_s {
	arc_t	arcn_arc;
	uint	arcn_code;			/* Notification code. */
} arcn_t;
#define	arcn_cmd		arcn_arc.arc_cmd
#define	arcn_name_offset	arcn_arc.arc_name_offset
#define	arcn_name_length	arcn_arc.arc_name_length

/** Client Notification Codes */
#define	AR_CN_BOGON	1
#define	AR_CN_ANNOUNCE	2

#endif	/* ARP_EXTERNAL_DEFINITIONS */

#define	ARP_REQUEST	1
#define	ARP_RESPONSE	2
#define	RARP_REQUEST	3
#define	RARP_RESPONSE	4

/** ARP Header */
typedef struct arh_s {
	uchar	arh_hardware[2];
	uchar	arh_proto[2];
	uchar	arh_hlen;
	uchar	arh_plen;
	uchar	arh_operation[2];
	/* The sender and target hw/proto pairs follow */
} arh_t;
#define	ARH_FIXED_LEN	8

/* ARP Cache Entry */
typedef struct ace_s {
	struct ace_s * ace_next;	/* Hash chain next pointer */
	struct ace_s ** ace_ptpn;	/* Pointer to previous next */
	struct arl_s * ace_arl;		/* Associated arl */
	uint	ace_proto;		/* Protocol for this ace */
	uint	ace_flags;
	uchar	* ace_proto_addr;
	uint	ace_proto_addr_length;
	uchar	* ace_proto_mask;	/* Mask for matching addr */
	uchar	* ace_proto_extract_mask; /* For mappings */
	uchar	* ace_hw_addr;
	uint	ace_hw_addr_length;
	uint	ace_hw_extract_start;	/* For mappings */
	mblk_t	* ace_mp;	/* mblk we are in */
	uint	ace_query_count;
	mblk_t	* ace_query_mp;		/* Head of outstanding query chain */
	mblk_t	* ace_query_mp_tail;	/* Tail of outstanding query chain */
	uint	ace_resend_cnt;		/* # probes|announcements to resend */
	uint	ace_resend_interval;	/* Milliseconds between resends. */
	uint	ace_resolved_time;	/* Time in seconds we verified addr */
	uint	ace_lastdefend_time;	/* Time_in_secs we last defended addr */
#ifdef	ACE_XTRA
	ACE_XTRA
#endif
} ace_t;
 
#define	ACE_HASH_TBL_COUNT	256

#ifndef ARP_SNAP_HEADER_SIZE
#define	ARP_SNAP_HEADER_SIZE	5
#endif

/* ARL Structure, one per link level device */
typedef struct arl_s {
	struct arl_s	* arl_next;		/* ARL chain at arl_g_head */
	queue_t		* arl_rq;		/* Read queue pointer */
	queue_t		* arl_wq;		/* Write queue pointer */
	int		arl_ppa;		/* DL_ATTACH parameter */
	int		arl_index;		/* ifIndex */
	uchar		* arl_arp_addr;		/* multicast address to use */
	uchar		* arl_hw_addr;		/* Our hardware address */
	uint		arl_hw_addr_length;
	uint		arl_arp_hw_type;	/* Our hardware type */
	uchar		* arl_name;		/* Lower level name */
	uint		arl_name_length;
	mblk_t		* arl_xmit_template;	/* DL_UNITDATA_REQ template */
	uint		arl_xmit_template_addr_offset;
	uint		arl_xmit_template_sap_offset;
	uint		arl_xmit_template_sap_length;
	mblk_t		* arl_unbind_mp;
	mblk_t		* arl_bind_mp;
	uint		arl_provider_style;	/* From DL_INFO_ACK */
	uchar		arl_mac_sap[4];		/* Network/big-endian order */
	uint		arl_min_sdu;		/* From DL_INFO_ACK */
	uint
			arl_bind_pending : 1,
			arl_unbind_pending : 1,
			arl_subs_bind_needed : 1,
			arl_use_snap_sap : 1,
			
			arl_bogon_notification : 1,
			arl_attach_done : 1,
			arl_up : 1,
			
			arl_pad : 25;
	mblk_t		* arl_queue;		/* Delayed due to pending ops */
	uchar		arl_snap[ARP_SNAP_HEADER_SIZE];
#ifdef	ARL_XTRA
	ARL_XTRA
#endif
} arl_t;

#endif	/*_ARP_*/
