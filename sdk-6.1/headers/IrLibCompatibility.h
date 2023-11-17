/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: IrLibCompatibility.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef __IRLIB_H__
#define __IRLIB_H__

#include <PalmTypes.h>

// name of Ir library
#define irLibName "IrDA Library"

// Specific scheme for IR exg lib
#define exgIrObexScheme			"_irobex"

// Feature Creators and numbers, for use with the FtrGet() call. This
//  feature can be obtained to get the current version of the Ir Library
#define		irFtrCreator			sysFileCIrLib
#define		irFtrNumVersion		0				// get version of Net Library
			// 0xMMmfsbbb, where MM is major version, m is minor version
			// f is bug fix, s is stage: 3-release,2-beta,1-alpha,0-development,
			// bbb is build number for non-releases 
			// V1.12b3   would be: 0x01122003
			// V2.00a2   would be: 0x02001002
			// V1.01     would be: 0x01013000


// Options values for IrOpen
// BDDDxxxx xxxxxxxx xxxxxxxx xxxSSSSS
// Where B=background mode, DDD=disconnect timeout, SSSSS=speed
#define irOpenOptBackground		0x80000000			// Unsupported background task use
#define irOpenOptDisconnect3	0x10000000			// sets amount of time in seconds
#define irOpenOptDisconnect8	0x20000000			// after no activity from other
#define irOpenOptDisconnect12	0x30000000			// device before disconnect is
#define irOpenOptDisconnect16	0x40000000			// initiated.
#define irOpenOptDisconnect20	0x50000000			// default is 40 secs
#define irOpenOptDisconnect25	0x60000000
#define irOpenOptDisconnect30	0x70000000
#define irOpenOptDisconnect40	0x00000000

#define irOpenOptSpeed115200	0x3F
#define irOpenOptSpeed57600		0x1F
#define irOpenOptSpeed38400		0x0F
#define irOpenOptSpeed19200		0x07
#define irOpenOptSpeed9600		0x03



/****************************************************************************
 *
 * Types and Constants
 *
 ****************************************************************************/

/* Maximum size of packet that can be sent at connect time (ConnectReq or
 * ConnectRsp) for IrLMP and Tiny TP connections. 
 */
#define IR_MAX_CON_PACKET     60
#define IR_MAX_TTP_CON_PACKET 52
#define IR_MAX_TEST_PACKET    376
#define IR_MAX_DEVICE_INFO    23

/* Size of the device list used in discovery process 
 */
#define IR_DEVICE_LIST_SIZE 6

/*---------------------------------------------------------------------------
 * 
 * Maximum size of the XID info field used in a discovery frame. The XID
 * info field contains the device hints and nickname. 
 */
#define IR_MAX_XID_LEN   23


/* Maximum allowed LSAP in IrLMP
 */
#define IR_MAX_LSAP       0x6f

/* The following are used to access the hint bits in the first byte
 * of the Device Info field of an XID frame (IrDeviceInfo).
 */
#define IR_HINT_PNP       0x01
#define IR_HINT_PDA       0x02
#define IR_HINT_COMPUTER  0x04
#define IR_HINT_PRINTER   0x08
#define IR_HINT_MODEM     0x10
#define IR_HINT_FAX       0x20
#define IR_HINT_LAN       0x40
#define IR_HINT_EXT       0x80

/* The following are used to access the hint bits in the second byte
 * of the Device Info field of an XID frame (IrDeviceInfo). Note 
 * that LM_HINT_EXT works for all hint bytes.
 */
#define IR_HINT_TELEPHONY 0x01
#define IR_HINT_FILE      0x02
#define IR_HINT_IRCOMM    0x04
#define IR_HINT_MESSAGE   0x08
#define IR_HINT_HTTP      0x10
#define IR_HINT_OBEX      0x20


/*---------------------------------------------------------------------------
 *
 * Status of a stack operation or of the stack.
 */
typedef uint8_t IrStatus;

#define IR_STATUS_SUCCESS        0  /* Successful and complete */
#define IR_STATUS_FAILED         1  /* Operation failed */
#define IR_STATUS_PENDING        2  /* Successfully started but pending */
#define IR_STATUS_DISCONNECT     3  /* Link disconnected */
#define IR_STATUS_NO_IRLAP       4  /* No IrLAP Connection exists */
#define IR_STATUS_MEDIA_BUSY     5  /* IR Media is busy */
#define IR_STATUS_MEDIA_NOT_BUSY 6  /* IR Media is not busy */
#define IR_STATUS_NO_PROGRESS    7  /* IrLAP not making progress */
#define IR_STATUS_LINK_OK        8  /* No progress condition cleared */

/*---------------------------------------------------------------------------
 *
 * Character set for user strings. These are definitions for the character
 * set in Nicknames and in IAS attributes of type User String.
 */
typedef uint8_t IrCharSet;

#define IR_CHAR_ASCII       0
#define IR_CHAR_ISO_8859_1  1
#define IR_CHAR_ISO_8859_2  2
#define IR_CHAR_ISO_8859_3  3
#define IR_CHAR_ISO_8859_4  4
#define IR_CHAR_ISO_8859_5  5
#define IR_CHAR_ISO_8859_6  6
#define IR_CHAR_ISO_8859_7  7
#define IR_CHAR_ISO_8859_8  8
#define IR_CHAR_ISO_8859_9  9
#define IR_CHAR_UNICODE     0xff
 
/*---------------------------------------------------------------------------
 *
 * All indication and confirmations are sent to the IrLMP/TTP connections
 * through one callback function. The types of the events passed are
 * defined below. Applications should ignore events listed as "reserved"
 * as well as events not listed at all, since we may add more events in
 * future versions of the IR library.
 */
typedef uint8_t IrEvent;

#define LEVENT_LM_CON_IND     0
#define LEVENT_LM_DISCON_IND  1
#define LEVENT_DATA_IND       2
#define LEVENT_PACKET_HANDLED 3
#define LEVENT_LAP_CON_IND    4
#define LEVENT_LAP_DISCON_IND 5
#define LEVENT_DISCOVERY_CNF  6
#define LEVENT_LAP_CON_CNF    7
#define LEVENT_LM_CON_CNF     8
#define LEVENT_STATUS_IND     9
#define LEVENT_TEST_IND       10
#define LEVENT_TEST_CNF       11
#define LEVENT_LM_SEND_IND    13	// added in Palm OS 4.0



/****************************************************************************
 *
 * IAS Types and Constants
 *
 ****************************************************************************/

/* Maximum size of a query that observes the IrDA Lite rules
 */
#define IR_MAX_QUERY_LEN 61

/* Maximum values for IAS fields. IR_MAX_IAS_NAME is the maximum allowable
 * size for IAS Object names and Attribute names.
 */
#define IR_MAX_IAS_NAME            60
#define IR_MAX_ATTRIBUTES         255

/* Maximum size of an IAS attribute that fits within the IrDA Lite rules. 
 * Even though attribute values can be larger IrDA Lite highly recommends 
 * that the total size of an attribute value fit within one 64 byte packet 
 * thus, the allowable size is 56 bytes or less. This size is enforced by the 
 * code.
 */
#define IR_MAX_IAS_ATTR_SIZE       56

/* Type of the IAS entry. This is the value returned for type when parsing 
 * the results buffer after a successful IAS Query.
 */
#define IAS_ATTRIB_MISSING      0
#define IAS_ATTRIB_INTEGER      1
#define IAS_ATTRIB_OCTET_STRING 2
#define IAS_ATTRIB_USER_STRING  3
#define IAS_ATTRIB_UNDEFINED    0xff

/* Ias Return Codes. One of these values will be found in the IAS Query
 * structure in the retCode field after a successful IAS Query.
 */
#define IAS_RET_SUCCESS        0    /* Query operation is successful */
#define IAS_RET_NO_SUCH_CLASS  1    /* Query failed no such class exists */
#define IAS_RET_NO_SUCH_ATTRIB 2    /* Query failed no such attribute exists */
#define IAS_RET_UNSUPPORTED    0xff /* Query failed operation is unsupported */

 /* IAS Get Value By Class opcode number
  */
#define IAS_GET_VALUE_BY_CLASS 4

// Macros used in accessing ias structures
#define IasGetU16(ptr) (uint16_t)( ((uint8_t)(*((uint8_t*)ptr) << 8)) | \
                       ((uint16_t) (*((uint8_t*)ptr+1))))
#define IasGetU32(ptr) (uint32_t)( ((uint32_t)(*((uint8_t*)ptr)) << 24)   | \
                              ((uint32_t)(*((uint8_t*)ptr+1)) << 16) | \
                              ((uint32_t)(*((uint8_t*)ptr+2)) << 8)  | \
                              ((uint32_t)(*((uint8_t*)ptr+3))) )

/****************************************************************************
 *
 * Data Structures
 *
 ****************************************************************************/

/*---------------------------------------------------------------------------
 *
 * Packet Structure for sending IrDA packets.
 */
typedef struct _IrPacket {
    /* The buff field is used to point to a buffer of data to send and len
     * field indicates the number of bytes in buff.
     */
    uint8_t *		buff;
    uint16_t		len;

    /*==================  For Internal Use Only =======================
     *
     * The following is used internally by the stack and should not be
     * modified by the upper layer.
     *
     *==================================================================*/

	uint8_t           reserved1; /* Explicitly account for 32-bit alignment */
	IrStatus          status;    /* send result */
	struct _IrPacket *next;      /* Linked list */
} IrPacket;


/*---------------------------------------------------------------------------
 *
 * 32-bit Device Address
 */
typedef  union {
    uint8_t		u8[4];
    uint16_t	u16[2];
    uint32_t	u32;
} IrDeviceAddr;

/*---------------------------------------------------------------------------
 *
 * The information returned for each device discovered during discovery.
 * The maximum size of the xid field is 23. This holds the hints and
 * the nickname. 
 */
typedef  struct {
    IrDeviceAddr hDevice;            /* 32-bit address of device */
    uint8_t      len;                /* Length of xid */
    uint8_t      xid[IR_MAX_XID_LEN];/* XID information */
} IrDeviceInfo;

/*---------------------------------------------------------------------------
 *
 * List of Device Discovery info elements.
 */
typedef  struct {
  uint32_t      nItems;                   /* Number items in the list */
  IrDeviceInfo  dev[IR_DEVICE_LIST_SIZE]; /* Fixed size in IrDA Lite */
} IrDeviceList;

/*---------------------------------------------------------------------------
 *
 * Definition of IrConnect structure. This structure is used to manage an
 * IrLMP or Tiny TP connection.
 */
typedef struct {
    uint8_t         lLsap;      /* Local LSAP this connection will listen on */
    uint8_t         rLsap;      /* Remote Lsap */

    /*==================  For Internal Use Only =======================
     *
     * The following is used internally by the stack and should not be
     * modified by the user.
     *
     *==================================================================*/

	uint8_t        dataOff;		/* Amount of data less than IrLAP size */
	uint8_t        availCredit;	/* Amount of credit to give to peer */
	uint8_t        data[32];	/* Private data */
} IrConnect;

/*---------------------------------------------------------------------------
 *
 * Callback Parameter Structure is used to pass information from the stack
 * to the upper layer of the stack (application). Not all fields are valid
 * at any given time. The type of event determines which fields are valid.
 */
typedef struct {
    IrEvent       event;       /* Event causing callback */
    IrStatus      status;      /* Status of stack */
    uint16_t      rxLen;       /* Length of data in receive buffer */
    uint8_t *     rxBuff;      /* Receive buffer already advanced to app data */
    IrPacket*     packet;      /* Pointer to packet being returned */
    IrDeviceList* deviceList;  /* Pointer to discovery device list */
} IrCallBackParms;

/* The definitions for the callback function is given below. How the
 * callback function is used in conjuction with the stack functions is
 * given below in the Callback Reference.
 */
typedef void (*IrCallBack)(IrConnect*, IrCallBackParms*);


/****************************************************************************
 *
 * IAS Data Strucutres
 *
 ****************************************************************************/

/*---------------------------------------------------------------------------
 *
 * The LmIasAttribute is a strucutre that holds one attribute of an IAS 
 * object.
 */
typedef struct _IrIasAttribute {
    uint8_t *       name;      /* Pointer to name of attribute */
    size_t          len;       /* Length of attribute name */
    uint8_t *       value;     /* Hardcode value (see below) */
    size_t          valLen;    /* Length of the value. */
} IrIasAttribute;

/* The value field of the IrIasAttribute structure is a hard coded string 
 * which represents the actual bytes sent over the IR for the attribute 
 * value. The value field contains all the bytes which represent an
 * attribute value based on the transmission format described in section
 * 4.3 of the IrLMP specification. An example of a user string is given
 * below.
 *
 * User String:
 *   1 byte type,  1 byte char set, 1 byte length, length byte string
 *
 * Example of an user string "Hello World" in ASCII
 *
 * U8 helloString[] = {
 *    IAS_ATTRIB_USER_STRING,IR_CHAR_ASCII,11,
 *    'H','e','l','l','o',' ','W','o','r','l','d'
 * };            
 */

/*---------------------------------------------------------------------------
 *
 * The LmIasObject is storage for an IAS object managed by the local
 * IAS server.
 */
typedef struct _IrIasObject {
  uint8_t *        name;      /* Pointer to name of object */
  size_t           len;       /* Length of object name */

  uint16_t         objectID;  /* Object ID assigned by IrIAS_Add() */
  uint16_t         nAttribs;  /* Number of attributes */
  IrIasAttribute*  attribs;   /* A pointer to an array of attributes */

} IrIasObject;


/*---------------------------------------------------------------------------
 *
 * Forward declaration of a structure used for performing IAS Queries so
 * that a callback type can be defined for use in the structure.
 */
typedef struct _IrIasQuery IrIasQuery;
typedef void (*IrIasQueryCallBack)(IrStatus);

/*---------------------------------------------------------------------------
 *
 * Actual definition of the IrIasQuery structure.
 */
typedef struct _IrIasQuery
{
    /* Query fields. The query buffer contains the class name and class
     * attribute whose value is being queried it is as follows:
     *
     * 1 byte - Length of class name
     * "Length" bytes - class name
     * 1 byte - length of attribute name
     * "Length" bytes - attribute name
     *
     * queryLen - contains the total number of byte in the query
     */
    size_t    queryLen;       /* Total length of the query */
    uint8_t * queryBuf;       /* Points to buffer containing the query */

    /* Fields for the query result */
    size_t    resultBufSize;  /* Size of the result buffer */
    size_t    resultLen;      /* Actual number of bytes in the result buffer */
    uint32_t  offset;         /* Offset into results buffer */
    uint16_t  listLen;        /* Number of items in the result list */
    uint8_t   retCode;        /* Return code of operation */
    Boolean   overFlow;       /* Set TRUE if result exceeded result buffer size */
    uint8_t * result;         /* Pointer to buffer containing result */

    /* Pointer to callback function */
    IrIasQueryCallBack callBack;
} _IrIasQuery;


/****************************************************************************
 *
 * Function Reference
 *
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 *
 * Prototype:     status_t	IrOpenV50(uint32_t options)
 *
 * Description:   Open the Ir library. This allocates the global memory
 *                for the ir stack and reserves and system resources it
 *                requires. This must be done before any other ir libary
 *                calls are made.
 *
 * Parameters:    
 *
 *				  options - open options flags
 *
 *
 * Return Values: zero if no error or exgErrStackInit
 *
 */
status_t	IrOpenV50(uint32_t options);
					
/*---------------------------------------------------------------------------
 *
 * Prototype:     status_t	IrCloseV50()
 *
 * Description:   Close the Ir library. This releases the global memory
 *                for the ir stack and any system resources it uses.
 *                This must be called when an application is done with the
 *				  ir library.
 *
 * Parameters:    
 *
 * Return Values: zero if no error
 *
 */ 
status_t	IrCloseV50(void);

/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IrBindV50(IrConnect* con,
 *                                   IrCallback callBack)
 *
 * Description:   Obtain a local LSAP selector and register the connection
 *                with the protocol stack. This IrConnect structure will be
 *                initialized. Any values stored in the structure will be
 *                lost. The assigned LSAP will be in the lLsap field of con.
 *                The type of the connection will be set to IrLMP. The 
 *                IrConnect must be bound to the stack before it can be used.
 *
 * Parameters:    
 *				  con - pointer to IrConnect structure.
 *
 *                callBack - pointer to a callBack function that handles
 *                the indications and confirmation from the protocol stack.
 *
 * Return Values: IR_STATUS_SUCCESS - operation completed successfully.
 *                The assigned LSAP can be found in con->lLsap.
 *
 *                IR_STATUS_FAILED - the operation failed for one of the
 *                following reasons:
 *                    - con is already bound to the stack
 *                    - no room in the connection table
 */
IrStatus IrBindV50(IrConnect* con, IrCallBack callBack);

/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IrUnbindV50(IrConnect* con)
 *
 * Description:   Unbind the IrConnect structure from the protocol stack
 *                freeing it's LSAP selector.
 *
 * Parameters:     
 *
 *					con - pointer to IrConnect structure to unbind
 *
 * Return Values: IR_STATUS_SUCCESS - operation competed succesfully
 *
 *                IR_STATUS_FAILED - operation failed
 *                either because the IrConnect structure was not bound
 *                or the lLsap field contained an invalid number.
 */
IrStatus IrUnbindV50(IrConnect* con);


/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IrDiscoverReqV50(IrConnect* con)
 *
 * Description:   Start an IrLMP discovery process. The result will be
 *                signaled via the callBack function specified in the
 *                IrConnect structure with the event LEVENT_DISCOVERY_CNF.
 *                Only one discovery can be invoked at a time.
 *
 * Parameters:    
 *				   con - pointer to a bound IrConnect structure.
 *
 * Return Values: IR_STATUS_PENDING - operation is started successfully 
 *                result returned via callback.
 *
 *                IR_STATUS_MEDIA_BUSY - operation failed because the media 
 *                is busy. Media busy is caused by one of the following
 *                reasons:
 *                    - Other devices are using the IR medium. 
 *                    - A discovery process is already in progress 
 *                    - An IrLAP connection exists.
 *
 *                IR_STATUS_FAILED - operation failed
 *                because the IrConnect structure is not bound to the stack.
 */
IrStatus IrDiscoverReqV50(IrConnect* con);

/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IrConnectIrLapV50(IrDeviceAddr deviceAddr)
 *
 * Description:   Start an IrLAP connection. The result is signaled to all
 *                bound IrConnect structures via the callback function. The
 *                callback event is LEVENT_LAP_CON_CNF if successful or
 *                LEVENT_LAP_DISCON_IND if unsuccessful.
 *
 * Parameters::   
 *				  deviceAddr - 32-bit address of device to which connection
 *                should be made.
 *
 * Return Values: IR_STATUS_PENDING - operation started successfully and
 *                callback will be called with result.
 *
 *                IR_STATUS_MEDIA_BUSY - operation failed to start because
 *                the IR media is busy. Media busy is caused by one of the
 *                following reasons:
 *                    - Other devices are using the IR medium.
 *                    - An IrLAP connection already exists
 *                    - A discovery process is in progress
 */
IrStatus IrConnectIrLapV50(IrDeviceAddr deviceAddr);

/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IrDisconnectIrLapV50()
 *
 * Description:   Disconnect the IrLAP connection. When the IrLAP connection
 *                goes down the callback of all bound IrConnect structures
 *                is called with event LEVENT_LAP_DISCON_IND.
 *
 * Parameters:
 *
 * Return Values: IR_STATUS_PENDING - operation started successfully and
 *                the all bound IrConnect structures will be called back
 *                when complete.
 *
 *                IR_STATUS_NO_IRLAP - operation failed because no IrLAP
 *                connection exists.
 */
IrStatus IrDisconnectIrLapV50(void);

/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IrConnectReqV50(
 *                                          IrConnect* con, 
 *                                          IrPacket* packet, 
 *                                          UInt8 credit); 
 *
 * Description:   Request an IrLMP or TinyTP connection. The result is
 *                is signaled via the callback specified in the IrConnect
 *                structure. The callback event is LEVENT_LM_CON_CNF
 *                indicates that the connection is up and LEVENT_LM_DISCON_IND
 *                indicates that the connection failed. Before calling this
 *                function the fields in the con structure must be properly
 *                set.
 *
 * Parameters:    
 *				  con - pointer to IrConnect structure for handing the
 *                the connection. The rLsap field must contain the LSAP
 *                selector for the peer on the other device. Also the type
 *                of the connection must be set. Use IR_SetConTypeLMP() to
 *                set the type to an IrLMP conneciton or IR_SetConTypeTTP()
 *                to set the type to a Tiny TP connection.
 *
 *                packet - pointer to a packet that contains connection data. 
 *                Even if no connection data is needed the packet must point 
 *                to a valid IrPacket structure. The packet will be returned 
 *                via the callback with the LEVENT_PACKET_HANDLED event if no 
 *                errors occur. The maximum size of the packet is 
 *                IR_MAX_CON_PACKET for an IrLMP connection or 
 *                IR_MAX_TTP_CON_PACKET for a Tiny TP connection.
 *
 *                credit - initial amount of credit advanced to the other side. 
 *                Must be less than 127. It is ANDed with 0x7f so if it is 
 *                greater than 127 unexpected results will occur. This 
 *                parameter is ignored if the Connection is an IrLMP connection.
 *
 * Return Values: IR_STATUS_PENDING - operation has been started successfully
 *                and the result will be returned via the callback function with
 *                the event LEVENT_LM_CON_CNF if the connection is made or
 *                LEVENT_LM_DISCON_IND if connection fails. The packet is returned
 *                via the callback with the event LEVENT_PACKET_HANDLED.
 *
 *                IR_STATUS_FAILED - operation failed because of one of the
 *                reasons below. Note that the packet is
 *                available immediately:
 *                   - Connection is busy (already involved in a connection)
 *                   - IrConnect structure is not bound to the stack
 *                   - Packet size exceeds maximum allowed.
 *
 *                IR_STATUS_NO_IRLAP - operation failed because there is no
 *                IrLAP connection (the packet is available immediately).
 */
IrStatus IrConnectReqV50(IrConnect* con, IrPacket* packet, uint8_t credit) ;

/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IrConnectRspV50(
 *                                          IrConnect* con,
 *                                          IrPacket* packet, 
 *                                          UInt8 credit); 
 *
 * Description:   Accept an incoming connection that has been signaled via
 *                the callback with the event LEVENT_LM_CON_IND. IR_ConnectRsp
 *                can be called during the callback or later to accept
 *                the connection. The type of the connection must already have
 *                been set to IrLMP or Tiny TP before LEVENT_LM_CON_IND event.
 *
 * Parameters:    
 *
 *				  con - pointer to IrConnect structure to managed connection.
 *
 *                packet - pointer to a packet that contains connection data.
 *                Even if no connection data is needed the packet must point
 *                to a valid IrPacket structure. The packet will be returned
 *                via the callback with the LEVENT_PACKET_HANDLED event if no
 *                errors occur. The maximum size of the packet is
 *                IR_MAX_CON_PACKET for an IrLMP connection or 
 *                IR_MAX_TTP_CON_PACKET for a Tiny TP connection.
 *
 *                credit - initial amount of credit advanced to the other side. 
 *                Must be less than 127. It is ANDed with 0x7f so if it is 
 *                greater than 127 unexpected results will occur. This 
 *                parameter is ignored if the Connection is an IrLMP connection.
 *
 * Return Values: IR_STATUS_PENDING - response has been started successfully
 *                and the packet is returned via the callback with the event 
 *                LEVENT_PACKET_HANDLED.
 *
 *                IR_STATUS_FAILED - operation failed because of one of the
 *                reasons below . Note that the packet is
 *                available immediately:
 *                   - Connection is not in the proper state to require a
 *                     response.
 *                   - IrConnect structure is not bound to the stack
 *                   - Packet size exceeds maximum allowed.
 *
 *                IR_STATUS_NO_IRLAP - operation failed because there is no
 *                IrLAP connection (Packet is available immediately).
 */
IrStatus IrConnectRspV50(IrConnect* con, IrPacket* packet, uint8_t credit);

/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IR_DataReqV50(IrConnect* con, 
 *                                       IrPacket* packet);
 *
 * Description:   Send a data packet. The packet is owned by the stack until
 *                it is returned via the callback with event 
 *                LEVENT_PACKET_HANDLED. The largest packet that can be sent
 *                is found by calling IR_MaxTxSize().
 *
 * Parameters:    
 *				  con - pointer to IrConnect structure that specifies the
 *                connection over which the packet should be sent.
 *
 *                packet - pointer to a packet that contains data to send. 
 *                The packet should exceed the max size found with 
 *                IR_MaxTxSize().
 *
 * Return Values: IR_STATUS_PENDING - packet has been queued by the stack.
 *                The packet will be returned via the callback with event
 *                LEVENT_PACKET_HANDLED.
 *
 *
 *                IR_STATUS_FAILED - operation failed and packet is available
 *                immediately. Operation failed for one of the following
 *                reasons:
 *                  - IrConnect structure is not bound to the stack (error
 *                    checking only)
 *                  - packet exceeds the maximum size (error checking only)
 *                  - IrConnect does not represent an active connection
 */
IrStatus IrDataReqV50(IrConnect* con, IrPacket* packet);

/*---------------------------------------------------------------------------
 *
 * Prototype:     void IrAdvanceCreditV50(IrConnect* con, 
 *                                        uint8_t credit);
 *
 * Description:   Advance credit to the other side. The total amount of
 *                credit should not exceed 127. The credit passed by this
 *                function is added to existing available credit which is 
 *                the number that must not exceed 127. This function
 *                only makes sense for a Tiny TP connection. 
 *
 * Parameters:    con - pointer to IrConnect structure representing 
 *                connection to which credit is advanced.
 *
 *                credit - number of credit to advance.
 *
 * Return Values: void
 */
void IrAdvanceCreditV50(IrConnect* con, uint8_t credit);

/*---------------------------------------------------------------------------
 *
 * Prototype:     void IrLocalBusyV50(Boolean flag);
 *
 * Description:   Set the IrLAP local busy flag. If local busy is set to true
 *                then the local IrLAP layer will send RNR frames to the other
 *                side indicating it cannot receive any more data. If the
 *                local busy is set to false IrLAP is ready to receive frames.
 *                This function should not be used when using Tiny TP or when
 *                multiple connections exist. It takes affect the next time
 *                IrLAP sends an RR frame. If IrLAP has data to send the data
 *                will be sent first so it should be used carefully.
 *
 * Parameters:   
 *				  flag - value (true or false) to set IrLAP's local busy flag.
 *
 * Return Values: void
 */
void IrLocalBusyV50(Boolean flag);

/*---------------------------------------------------------------------------
 *
 * Prototype:     void IrSetConTypeTTPV50(IrConnect* con)
 *
 * Description:   Set the type of the connection to Tiny TP. This function
 *                must be called after the IrConnect structure is bound to 
 *                the stack.
 *
 * Parameters:    con - pointer to IrConnect structure.
 *
 * Return Values: IR_STATUS_SUCCESS - connection type updated
 *
 *                IR_STATUS_FAILED - connection type not updated, because
 *                    'con' is not bound to the stack, or is already
 *                    engaged in an active connection
 */
IrStatus IrSetConTypeTTPV50(IrConnect* con);

/*---------------------------------------------------------------------------
 *
 * Prototype:     void IrSetConTypeLMPV50(IrConnect* con)
 *
 * Description:   Set the type of the connection to IrLMP. This function
 *                must be called after the IrConnect structure is bound to
 *                the stack.
 *
 * Parameters:    con - pointer to IrConnect structure.
 *
 * Return Values: IR_STATUS_SUCCESS - connection type updated
 *
 *                IR_STATUS_FAILED - connection type not updated, because
 *                    'con' is not bound to the stack, or is already
 *                    engaged in an active connection
 */
IrStatus IrSetConTypeLMPV50(IrConnect* con);

/*---------------------------------------------------------------------------
 *
 * Prototype:     uint16_t IrMaxTxSizeV50(IrConnect* con);
 *
 * Description:   Returns the maximum size allowed for a transmit packet.
 *                The value returned is only valid for active connections. 
 *                The maximum size will vary for each connection and is based 
 *                on the negotiated IrLAP parameters and the type of the 
 *                connection.
 *
 * Parameters: 
 *				  con - pointer to IrConnect structure which represents
 *                an active connection.
 *
 * Return Values: Maxmum number of bytes for a transmit packet.
 */
uint16_t IrMaxTxSizeV50(IrConnect* con);

/*---------------------------------------------------------------------------
 *
 * Prototype:    IrMaxRxSizeV50(IrConnect* con);
 *
 * Description:   Returns the maximum size buffer that can be sent by the
 *                the other device. The value returned is only valid for
 *                active connections. The maximum size will vary for
 *                each connection and is based on the negotiated IrLAP
 *                parameters and the type of the connection.
 *
 * Parameters:    
 *
 *				  con - pointer to IrConnect structure which represents
 *                an active connection.
 *
 * Return Values: Maxmum number of bytes that can be sent by the other
 *                device (maximum bytes that can be received).
 */
uint16_t IrMaxRxSizeV50(IrConnect* con);

/*---------------------------------------------------------------------------
 *
 * Prototype:     Boolean IrIsNoProgressV50();
 *
 * Description:   Return true if IrLAP is not making progress otherwise
 *                return false (this is an optional function). 
 *
 * Parameters:    
 *
 * Return Values: true if IrLAP is not making progress, false otherwise.
 */
Boolean IrIsNoProgressV50(void);

/*---------------------------------------------------------------------------
 *
 * Prototype:     Boolean IrIsRemoteBusyV50()
 *
 * Description:   Return true if the other device's IrLAP is busy otherwise
 *                return false (this is an optional function).
 *
 * Parameters:    
 *
 * Return Values: true if the other device's IrLAP is busy, false otherwise.
 */
Boolean IrIsRemoteBusyV50(void);

/*---------------------------------------------------------------------------
 *
 * Prototype:     Boolean IrIsMediaBusyV50();
 *
 * Description:   Return true if the IR media is busy. Otherwise return false 
 *                (this is an optional function). 
 *
 * Parameters:    
 *
 * Return Values: true if IR media is busy, false otherwise.
 */
Boolean IrIsMediaBusyV50(void);

/*---------------------------------------------------------------------------
 *
 * Prototype:     Boolean IrIsIrLapConnectedV50();
 *
 * Description:   Return true if an IrLAP connection exists (this is an 
 *                optional function). Only available if IR_IS_LAP_FUNCS is 
 *                defined.
 *
 * Parameters:    
 *
 * Return Values: true if IrLAP is connected, false otherwise.
 */
Boolean IrIsIrLapConnectedV50(void);

/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IR_TestReqV50(IrDeviceAddr devAddr,
 *                                       IrConnect* con,
 *                                       IrPacket* packet) 
 *
 * Description:   Request a TEST command frame be sent in the NDM state. The 
 *                result is signaled via the callback specified in the 
 *                IrConnect structure. The callback event is LEVENT_TEST_CNF 
 *                and the status field indates the result of the operation.
 *                IR_STATUS_SUCCESS indicates success and IR_STATUS_FAILED 
 *                indicates no response was received. A packet must be passed
 *                containing the data to send in the TEST frame. The packet 
 *                is returned when the LEVENT_TEST_CNF event is given. 
 *
 *
 * Parameters:    
 *
 *                devAddr - device address of device where TEST will be
 *                sent. This address is not checked so it can be the
 *                broadcast address or 0.
 *
 *                con - pointer to IrConnect structure specifying the
 *                callback function to call to report the result.
 *
 *                packet - pointer to a packet that contains the data to
 *                send in the TEST command packet. The maximum size data
 *                that can be sent is IR_MAX_TEST_PACKET. Even if no
 *                data is to be sent a valid packet must be passed.
 *
 *
 * Return Values: IR_STATUS_PENDING - operation has been started successfully
 *                and the result will be returned via the callback function with
 *                the event LEVENT_TEST_CNF. This is also the indication 
 *                returning the packet.
 *
 *                IR_STATUS_FAILED - operation failed because of one of the
 *                reasons below. Note that the packet is
 *                available immediately:
 *                   - IrConnect structure is not bound to the stack
 *                   - Packet size exceeds maximum allowed.
 *
 *                IR_STATUS_MEDIA_BUSY - operation failed because the media is
 *                busy or the stack is not in the NDM state (the packet is 
 *                available immediately).
 */
IrStatus IrTestReqV50(IrDeviceAddr devAddr, IrConnect* con, IrPacket* packet);


/****************************************************************************
 *
 * Callback Reference
 *
 ****************************************************************************/

/*---------------------------------------------------------------------------
 *
 * The stack calls the application via a callback function stored in each
 * IrConnect structure. The callback function is called with a pointer to
 * the IrConnect structure and a pointer to a parameter structure. The
 * parameter structure contains an event field which indicates the reason 
 * the callback is called and other parameters which have meaning based
 * on the event.
 *
 * The meaning of the events are as follows:
 *
 * LEVENT_LM_CON_IND - Other device has initiated a connection. IR_ConnectRsp
 * should be called to accept the connection. Any data associated with the
 * connection request can be found using fields rxBuff and rxLen for the
 * data pointer and length respectively. 
 * 
 * LEVENT_LM_DISCON_IND - The IrLMP/Tiny TP connection has been disconnected.
 * Any data associated with the disconnect indication can be found using 
 * fields rxBuff and rxLen for the data pointer and length respectively. 
 * 
 * LEVENT_DATA_IND - Data has been received. The received data is accessed 
 * using fields rxBuff and rxLen; 
 * 
 * LEVENT_PACKET_HANDLED - A packet is being returned. A pointer to the 
 * packet exists in field packet. 
 * 
 * LEVENT_LAP_CON_IND - Indicates that the IrLAP connection has come up. The 
 * callback of all bound IrConnect structures is called. 
 *
 * LEVENT_LAP_DISCON_IND - Indicates that the IrLAP connection has gone 
 * down. This means that all IrLMP connections are also down. A callback 
 * with event LEVENT_LM_CON_IND will not be given. The callback function 
 * of all bound IrConnect structures is called.
 *
 * LEVENT_DISCOVERY_CNF - Indicates the completion of a discovery operation. 
 * The field deviceList points to the discovery list.
 * 
 * LEVENT_LAP_CON_CNF - The requested IrLAP connection has been made
 * successfully. The callback function of all bound IrConnect structures
 * is called.
 *
 * LEVENT_LM_CON_CNF - The requested IrLMP/Tiny TP connection has been made
 * successfully. Connection data from the other side is found using fields
 * rxBuff and rxLen.
 * 
 * LEVENT_STATUS_IND - Indicates that a status event from the stack has 
 * occured. The status field indicates the status generating the event. 
 * Possible statuses are as follows. Note this event is optional:
 *    IR_STATUS_NO_PROGRESS - means that IrLAP has no progress for 3 seconds
 *    threshold time (e.g. beam is blocked).
 *
 *    IR_STATUS_LINK_OK - indicates that the no progress condition has
 *    cleared.
 *
 *    IR_STATUS_MEDIA_NOT_BUSY - indicates that the IR media has 
 *    transitioned from busy to not busy.
 *
 * LEVENT_TEST_IND - Indicates that a TEST command frame has been received.
 * A pointer to the received data is in rxBuff and rxLen. A pointer to the
 * packet that will be sent in response to the test command is in the packet
 * field. The packet is currently setup to respond with the same data sent
 * in the command TEST frame. If different data is desired as a response
 * then modify the packet structure. This event is sent to the callback 
 * function in all bound IrConnect structures. The IAS connections ignore 
 * this event. 
 *
 * LEVENT_TEST_CNF - Indicates that a TEST command has completed. The status
 * field indicates if the test was successful. IR_STATUS_SUCCESS indicates
 * that operation was successful and the data in the test response can be
 * found by using the rxBuff and rxLen fields. IR_STATUS_FAILED indicates 
 * that no TEST response was received. The packet passed to perform the test 
 * command is passed back in the packet field and is now available (no 
 * separate packet handled event will occur).
 */
/* The following functions are used to extract U16 and U32 bit numbers
 * from an IAS result. Only IasGetU16 is used internal by the stack
 * but they are part of some of the IAS Query result macros. To enable
 * the function versions define IR_IAS_GET_AS_FUNC
 */


/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IrIAS_AddV50(IrIasObject* obj)
 *
 * Description:   Add an IAS Object to the IAS Database. The Object is
 *                is not copied so the memory for the object must exist
 *                for as long as the object is in the data base. The
 *                IAS database is designed to only allow objects with unique 
 *                class names. The error checking version checks for this.
 *                Class names and attributes names must not exceed 
 *                IR_MAX_IAS_NAME. Also attribute values must not exceed
 *                IR_MAX_IAS_ATTR_SIZE. 
 *
 * Parameters:    refNum - ir library reference number
 *
 *                obj - pointer to an IrIasObject structure.
 *
 * Return Values: IR_STATUS_SUCCESS - operation is successful.
 *
 *                IR_STATUS_FAILED - operation failed for one of the
 *                following reasons:
 *                  - No space in the data base (see irconfig.h to 
 *                    increase the size of the IAS database).
 *                  - An entry with the same class name already exists.
 *                    Error check only.
 *                  - The attributes of the object violate the IrDA Lite
 *                    rules (attribute name exceeds IR_MAX_IAS_NAME or
 *                    attribute value exceeds IR_MAX_IAS_ATTR_SIZE).
 *                    Error check only.
 *                  - The class name exceeds IR_MAX_IAS_NAME. Error check
 *                    only
 */
IrStatus IrIAS_AddV50(IrIasObject* obj);


/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IRIAS_RemoveV50(IrIasObject* obj)
 *
 * Description:   Remove an IAS object from the IAS database.
 *
 * Parameters:    obj - pointer to an IrIasObject structure.
 *
 * Return Values: Status of the operation. :
 *				  IR_STATUS_SUCCESS - operation is successful.
 *
 *                IR_STATUS_FAILED - operation failed because the specified
 *                object could not be found into IAS database.
 */
IrStatus IrIAS_RemoveV50(IrIasObject* obj);


/*---------------------------------------------------------------------------
 *
 * Prototype:     IrStatus IrIAS_QueryV50(IrIasQuery* token)
 *
 * Description:   Make an IAS query of another devices IAS database. An IrLAP
 *                connection must exist to the other device. The IAS query
 *                token must be initialized as described below. The result is
 *                signaled by calling the callback function whose pointer
 *                exists in the IrIasQuery structure. Only one Query can be
 *                made at a time.
 *
 * Parameters:    refNum - ir library reference number
 *
 *                token - pointer to an IrIasQuery structure initialized
 *                as follows:
 *                   - pointer to a callback function in which the result will
 *                     signaled.
 *                   - result points to a buffer large enough to hold the
 *                     result of the query.
 *                   - resultBufSize is set to the size of the result buffer.
 *                   - queryBuf must point to a valid query.
 *                   - queryLen is set to the number of bytes in queryBuf.
 *                     The length must not exceed IR_MAX_QUERY_LEN.
 *
 * Return Values: IR_STATUS_PENDING - operation is started successfully and
 *                the result will be signaled via the calback function.
 *
 *                IR_STATUS_FAILED - operation failed for one of the 
 *                following reasons (Error check only):
 *                   - The query exceeds IR_MAX_QUERY_LEN.
 *                   - The result field of token is 0.
 *                   - The resultBuffSize field of token is 0.
 *                   - The callback field of token is 0.
 *                   - A query is already in progress.
 *
 *                IR_STATUS_NO_IRLAP - operation failed because there is no
 *                IrLAP connection.
 */
IrStatus IrIAS_QueryV50(IrIasQuery* token);

/*---------------------------------------------------------------------------
 *
 * Below are some functions and macros for parsing the results buffer
 * after a successfull IAS Query.
 */

/*---------------------------------------------------------------------------
 *
 * Prototype:     void IrIAS_StartResultV50(IrIasQuery* token)
 *
 * Description:   Put the internal pointer to the start of the
 *                result buffer.
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: void
 */
#define IrIAS_StartResultV50(t) ((t)->offset = 0)

/*---------------------------------------------------------------------------
 *
 * Prototype:     U16 IRIAS_GetObjectIDV50(IrIasQuery* token)
 *
 * Description:   Return the unique object ID of the current result item. 
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: object ID
 */
#define IrIAS_GetObjectIDV50(t) IasGetU16((t)->result + (t)->offset)

/*---------------------------------------------------------------------------
 *
 * Prototype:     U8 IrIAS_GetTypeV50(IrIasQuery* token)
 *
 * Description:   Return the type of the current result item
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: Type of result item such as IAS_ATTRIB_INTEGER, 
 *                IAS_ATTRIB_OCTET_STRING or IAS_ATTRIB_USER_STRING.
 */
#define IrIAS_GetTypeV50(t) ((t)->result[(t)->offset + 2])

/*---------------------------------------------------------------------------
 *
 * Prototype:     U32 IrIAS_GetIntegerV50(IrIasQuery* token)
 *
 * Description:   Return an integer value assuming that the current result 
 *                item is of type IAS_ATTRIB_INTEGER (call IRIAS_GetType() to 
 *                determine the type of the current result item). 
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: Integer value.
 */
#define IrIAS_GetIntegerV50(t) IasGetU32((t)->result + (t)->offset + 3)


/*---------------------------------------------------------------------------
 *
 * Prototype:     U8 IrIAS_GetIntLsapV50(IrIasQuery* token)
 *
 * Description:   Return an integer value that represents an LSAP assuming 
 *                that the current result item is of type IAS_ATTRIB_INTEGER 
 *                (call IRIAS_GetType() to determine the type of the current 
 *                result item). Usually integer values returned in a query
 *                are LSAP selectors.
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: Integer value.
 */
#define IrIAS_GetIntLsapV50(t) ((t)->result[(t)->offset + 6])

/*---------------------------------------------------------------------------
 *
 * Prototype:     U16 IrIAS_GetOctetStringLenV50(IrIasQuery* token)
 *
 * Description:   Get the length of an octet string assuming that the current
 *                result item is of type IAS_ATTRIB_OCTET_STRING (call 
 *                IRIAS_GetType() to determine the type of the current result 
 *                item).
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: Length of octet string
 */
#define IrIAS_GetOctetStringLenV50(t) IasGetU16((t)->result + (t)->offset + 3)

/*---------------------------------------------------------------------------
 *
 * Prototype:     U8* IrIAS_GetOctetStringV50(IrIasQuery* token)
 *
 * Description:   Return a pointer to an octet string assuming that the
 *                current result item is of type IAS_ATTRIB_OCTET_STRING (call 
 *                IRIAS_GetType() to determine the type of the current result 
 *                item).
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: pointer to octet string
 */
#define IrIAS_GetOctetStringV50(t) ((t)->result + (t)->offset + 5)

/*---------------------------------------------------------------------------
 *
 * Prototype:     U8 IrIAS_GetUserStringLenV50(IrIasQuery* token)
 *
 * Description:   Return the length of a user string assuming that the
 *                current result item is of type IAS_ATTRIB_USER_STRING (call
 *                IRIAS_GetType() to determine the type of the current result 
 *                item).
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: Length of user string
 */
#define IrIAS_GetUserStringLenV50(t) ((t)->result[(t)->offset + 4])

/*---------------------------------------------------------------------------
 *
 * Prototype:     IrCharSet IrIAS_GetUserStringCharSetV50(IrIasQuery* token)
 *
 * Description:   Return the character set of the user string assuming that
 *                the current result item is of type IAS_ATTRIB_USER_STRING 
 *                (call IRIAS_GetType() to determine the type of the current 
 *                result item).
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: Character set
 */
#define IrIAS_GetUserStringCharSetV50(t) ((t)->result[(t)->offset + 3])

/*---------------------------------------------------------------------------
 *
 * Prototype:     U8* IrIAS_GetUserStringV50(IrIasQuery* token)
 *
 * Description:   Return a pointer to a user string assuming that the
 *                current result item is of type IAS_ATTRIB_USER_STRING (call
 *                IRIAS_GetType() to determine the type of the current result 
 *                item).
 *
 * Parameters:    token - pointer to an IrIasQuery structure
 *
 * Return Values: Pointer to result string
 */
#define IrIAS_GetUserStringV50(t) ((t)->result + (t)->offset + 5)


 /*---------------------------------------------------------------------------
 *
 * Prototype:     UInt8 *IrIAS_NextV50(IrIasQuery* token)
 *
 * Description:   Move the internal pointer to the next result item. This
 *                function returns a pointer to the start of the next result
 *                item. If the poiinter is 0 then there are no more result
 *                items. Only available if IR_IAS_NEXT is defined.
 *
 * Parameters:    refNum - library reference number
 *
 *				 token - pointer to an IrIasQuery structure
 *
 * Return Values: Pointer to the next result item or 0 if no more items.
 */
uint8_t * IrIAS_NextV50(IrIasQuery* token);


/****************************************************************************
 *
 * IAS Callback Reference
 *
 ****************************************************************************/

/*---------------------------------------------------------------------------
 *
 * The result of IAS query is signaled by calling the callback function
 * pointed to by the callBack field of IrIasQuery structure. The callback
 * has the following prototype:
 *
 *  void callBack(IrStatus);
 *
 * The callback is called with a status as follows:
 *
 *    IR_STATUS_SUCCESS - the query operation finished successfully and
 *    the results can be parsed
 *
 *    IR_STATUS_DISCONNECT - the link or IrLMP connection was disconnected
 *    during the query so the results are not valid.

=========================================================================== */

/****************************************************************************
 *
 * The following functions cannot reasonably be supported within the
 * new IrDA architecture, and as such, have been removed completely:
 *
 * IrStatus IrSetDeviceInfo(UInt8 *info,UInt8 len);
 * IrStatus IrIAS_SetDeviceName(UInt8 *name, UInt8 len);
 *
 ****************************************************************************/


#ifdef __cplusplus 
}
#endif

#endif  // IR_LIB_H


