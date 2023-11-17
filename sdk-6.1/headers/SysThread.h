/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SysThread.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  High-level C API for creating and managing threads.
 *
 *****************************************************************************/

#ifndef _SYSTHREAD_H_
#define _SYSTHREAD_H_

// Include elementary types
#include <PalmTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------
// THREAD CONSTANTS
// -------------------------------------------------------

// Standard thread priority levels.  ALWAYS use these constants
// unless you have a very, very good reason to use a special value.
#define sysThreadPriorityLowered			100		// Background processing or CPU hogs
#define sysThreadPriorityNormal				80		// Default priority for event handling
#define sysThreadPriorityBestApp				80		// Highest priority for application process and any thread created by it
#define sysThreadPriorityRaised				70		// Increased priority for user operations
#define sysThreadPriorityTransaction		65		// UI transactions
#define sysThreadPriorityDisplay			60		// Drawing to screen
#define sysThreadPriorityUrgentDisplay		50		// Event collection and dispatching
#define sysThreadPriorityRealTime			40		// Time critical such as audio
#define sysThreadPriorityBestUser			30		// Highest priority for user programs
#define sysThreadPriorityBestSystem			5		// Highest priority for system programs

// A few synonyms.  Should go away.
#define sysThreadPriorityLow				sysThreadPriorityLowered
#define sysThreadPriorityHigh				sysThreadPriorityRealTime

// Standard thread stack sizes.
#define sysThreadStackBasic					4*1024	// Minimum stack size for basic stuff.
#define sysThreadStackUI					8*1024	// Minimum stack size for threads that do UI.

// Timeout flags
typedef enum {
	P_WAIT_FOREVER              = 0x0000,
	P_POLL                      = 0x0300,
	P_RELATIVE_TIMEOUT          = 0x0100,
	P_ABSOLUTE_TIMEOUT          = 0x0200
} timeoutFlagsEnum_t;
typedef uint16_t timeoutFlags_t; /* a combination of timeoutFlagsEnum_t and timebase_t flags */

// -------------------------------------------------------
// THREAD SUPPORT
// -------------------------------------------------------

// Atomic operations.
int32_t SysAtomicInc32(int32_t volatile *ioOperandP);
int32_t SysAtomicDec32(int32_t volatile *ioOperandP);
int32_t SysAtomicAdd32(int32_t volatile *ioOperandP, int32_t iAddend);
uint32_t SysAtomicAnd32(uint32_t volatile *ioOperandP, uint32_t iValue);
uint32_t SysAtomicOr32(uint32_t volatile *ioOperandP, uint32_t iValue);
uint32_t SysAtomicCompareAndSwap32(uint32_t volatile *ioOperandP, uint32_t iOldValue,
				 uint32_t iNewValue);

// Time
nsecs_t SysGetRunTime(void);

// Thread Specific Data
typedef uint32_t SysTSDSlotID;
typedef void (SysTSDDestructorFunc)(void*);
enum { sysTSDAnonymous=0 }; // symbolic value for iName in unnamed slots
status_t	SysTSDAllocate(SysTSDSlotID *oTSDSlot, SysTSDDestructorFunc *iDestructor, uint32_t iName);
status_t	SysTSDFree(SysTSDSlotID tsdslot);
void		*SysTSDGet(SysTSDSlotID tsdslot);
void		SysTSDSet(SysTSDSlotID tsdslot, void *iValue);

// Thread Finalizers
typedef uint32_t SysThreadExitCallbackID;
typedef void (SysThreadExitCallbackFunc)(void*);
status_t	SysThreadInstallExitCallback(	SysThreadExitCallbackFunc *iExitCallbackP,
											void *iCallbackArg,
											SysThreadExitCallbackID *oThreadExitCallbackId);
status_t	SysThreadRemoveExitCallback(SysThreadExitCallbackID iThreadCallbackId);

// Lightweight locking
#define sysCriticalSectionInitializer NULL
typedef void * SysCriticalSectionType;

#if TARGET_HOST == TARGET_HOST_PALMOS
void SysCriticalSectionEnter(SysCriticalSectionType *iCS);
void SysCriticalSectionExit(SysCriticalSectionType *iCS);
#endif //TARGET_HOST == TARGET_HOST_PALMOS

// Lightweight condition variables
#define sysConditionVariableInitializer NULL
typedef void * SysConditionVariableType;

#if TARGET_HOST == TARGET_HOST_PALMOS
void SysConditionVariableWait(SysConditionVariableType *iCV, SysCriticalSectionType *iOptionalCS);
void SysConditionVariableOpen(SysConditionVariableType *iCV);
void SysConditionVariableClose(SysConditionVariableType *iCV);
void SysConditionVariableBroadcast(SysConditionVariableType *iCV);
#endif //TARGET_HOST == TARGET_HOST_PALMOS

// Counting semaphores
enum { sysSemaphoreMaxCount = 0xffff };
status_t	SysSemaphoreCreateEZ(uint32_t initialCount, SysHandle* outSemaphore);
status_t	SysSemaphoreCreate(	uint32_t initialCount, uint32_t maxCount, uint32_t flags,
								SysHandle* outSemaphore);
status_t	SysSemaphoreDestroy(SysHandle semaphore);
status_t	SysSemaphoreSignal(SysHandle semaphore);
status_t	SysSemaphoreSignalCount(SysHandle semaphore, uint32_t count);
status_t	SysSemaphoreWait(	SysHandle semaphore,
								timeoutFlags_t iTimeoutFlags, nsecs_t iTimeout);
status_t	SysSemaphoreWaitCount(	SysHandle semaphore,
									timeoutFlags_t iTimeoutFlags, nsecs_t iTimeout,
									uint32_t count);

// -------------------------------------------------------
// THREAD GROUPS
// -------------------------------------------------------

// Thread groups.  These are a convenience provided by the system
// library to wait for one or more threads to exit.  It is useful
// for unloading libraries that have spawned their own threads.
// Note that destroying a thread group implicitly waits for all
// threads in that group to exit.

struct SysThreadGroupTag;
typedef struct SysThreadGroupTag SysThreadGroupType;
typedef SysThreadGroupType*	SysThreadGroupHandle;

SysThreadGroupHandle	SysThreadGroupCreate(void);
status_t				SysThreadGroupDestroy(SysThreadGroupHandle group);
status_t				SysThreadGroupWait(SysThreadGroupHandle group);

// Use this if you don't want the thread in a group.
#define			sysThreadNoGroup	NULL


// -------------------------------------------------------
// THREAD CREATION
// -------------------------------------------------------

// Create a new thread.  Arguments are:
//	name			-- first four letters are used for thread ID.
//	func			-- entry point of thread.
//	argument		-- data passed to above.
//	outThread		-- outgoing thread key ID.
//
// The thread's priority is sysThreadPriorityNormal and its
// stack size is sysThreadStackUI.
//
// The returned SysHandle is owned by the newly created thread, and
// we be freed when that thread exits.  It is the same SysHandle as
// the thread uses for its local identity in its TSD, so you can
// use this as a unique identity for the thread.
typedef void (SysThreadEnterFunc) (void *);
status_t	SysThreadCreateEZ(	const char *name,
								SysThreadEnterFunc *func, void *argument,
								SysHandle* outThread);

// Like SysThreadCreateEZ(), but with more stuff:
//	group			-- thread group, normally sysThreadNoGroup.
//	priority		-- pick a constant from above.
//	stackSize		-- bytes available for stack.
status_t	SysThreadCreate(SysThreadGroupHandle group, const char *name,
							uint8_t priority, uint32_t stackSize,
							SysThreadEnterFunc *func, void *argument,
							SysHandle* outThread);

// Start a thread that was created above.  Arguments are:
//	thread			-- as returned by SysThreadCreate().
//
// If this function fails, the thread handle and all
// associated resources will be deallocated.
status_t	SysThreadStart(SysHandle thread);

// Return the handle for the calling thread.
SysHandle	SysCurrentThread(void);

// Call this function to have the current thread exit.  The thread
// MUST have been created with SysThreadCreate().
void		SysThreadExit(void);

// Go to sleep for the given amount of time.
status_t	SysThreadDelay(nsecs_t timeout, timeoutFlags_t flags);

// Start and stop threads.
status_t	SysThreadSuspend(SysHandle thread);
status_t	SysThreadResume(SysHandle thread);

// Priority control.
status_t	SysThreadChangePriority(SysHandle thread, uint8_t priority);

#ifdef __cplusplus
}	// extern "C"
#endif

#endif // _SYSTHREAD_H_
