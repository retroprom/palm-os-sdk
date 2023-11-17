/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ObexServiceLib.h 
 *
 * Release: Palm OS 6.0
 *
 * Description: Contains function declarations.
 *****************************************************************************/

#ifndef _OBEXSERVICELIB_H_
#define _OBEXSERVICELIB_H_

#include <PalmTypes.h>

#define exgObexInterfaceIRDA   0
#define exgObexInterfaceBluetooth 1

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
	status_t ObexEnableReceive( uint32_t obexInterface, Boolean enable, Boolean SaveSetting );
	status_t ObexIsReceiveEnabled( uint32_t obexInterface, Boolean *enabledP );
#ifdef __cplusplus
}
#endif //__cplusplus


#endif //_OBEXSERVICELIB_H_
