/******************************************************************************
 *
 *	Copyright (c) 2004 PalmSource, Inc. or its subsidiaries.
 *	All rights reserved.
 *
 *	File: DALLibEntryNums.h
 *
 *	Description:
 *		This file is automatically generated by the
 *		PalmSource Shared Library Generator.
 *
 *	History:
 *		Generated on: Tue Aug 31 14:09:55 2004
 *****************************************************************************/
#ifndef __DALLIBENTRYNUMS_H__	// Avoid multiple inclusion
#define __DALLIBENTRYNUMS_H__

// System header file
#include <PalmTypes.h>

enum dallibEntryNumTag {
	dallibEntryNumRALLinkerStubEnum = 0,
	dallibEntryNumRALInitNewProcessEnum,
	dallibEntryNumRALGetModuleInfoEnum,
	dallibEntryNumRALLoadModuleWithRPCEnum,
	dallibEntryNumRALUnloadModuleWithRPCEnum,
	dallibEntryNumRALLoadBootModuleEnum,
	dallibEntryNumRALGetEntryAddressesEnum,
	dallibEntryNumRALGetROMHeaderEnum,
	dallibEntryNumRALGetNVParamsEnum,
	dallibEntryNumRALGetVectorTableEnum,
	dallibEntryNumRALRegisterPatchEnum,
	dallibEntryNumRALLibPreDmInitEnum,
	dallibEntryNumRALLibPostDmInitEnum,
	dallibEntryNumRALFixupModuleEnum,
	dallibEntryNumRALRelocEnum,
	dallibEntryNumRALCheckModuleEnum,
	dallibEntryNumRALOnModuleLoadedEnum,
	dallibEntryNumRALOnModuleUnloadedEnum,
	dallibEntryNumRALIsModuleLoadedEnum,
	dallibEntryNumRALGetLoadedModuleInfoEnum,
	dallibEntryNumRALSetModuleDatabaseEnum,
	dallibEntryNumRALGetModuleGlobalsEnum,
	dallibEntryNumRALBroadcastActionCodeEnum,
	dallibEntryNumRALUnloadUnloadableEnum,
	dallibEntryNumRALLockBootResourcesEnum,
	dallibEntryNumRALCheckBootModuleEnum,
	dallibEntryNumRALLibOnProcessDestroyedEnum,
	dallibEntryNum_RALInstallPostLoadHookEnum,
	dallibEntryNumErrSetJumpEnum,
	dallibEntryNumErrLongJumpEnum,
	dallibEntryNumErrExceptionListAppendEnum,
	dallibEntryNumErrExceptionListRemoveEnum,
	dallibEntryNumErrThrowWithAddressEnum,
	dallibEntryNumErrThrowWithHandlerEnum,
	dallibEntryNumErrExceptionListGetByThreadIDEnum,
	dallibEntryNumDrvMessageEnum,
	dallibEntryNumDrvVPrintfEnum,
	dallibEntryNumDrvPrintfEnum,
	dallibEntryNumDrvBreakEnum,
	dallibEntryNumDrvGetCharEnum,
	dallibEntryNumDrvIsPresentEnum,
	dallibEntryNumDrvOutputSyncEnum,
	dallibEntryNumDrvFatalErrorInContextEnum,
	dallibEntryNumSysAtomicAdd32Enum,
	dallibEntryNumSysAtomicAnd32Enum,
	dallibEntryNumSysAtomicOr32Enum,
	dallibEntryNumSysAtomicCompareAndSwap32Enum,
	dallibEntryNumSysAtomicAdd64Enum,
	dallibEntryNumSysAtomicCompareAndSwap64Enum,
	dallibEntryNumSysGetRunTimeEnum,
	dallibEntryNumSysTSDAllocateEnum,
	dallibEntryNumSysTSDFreeEnum,
	dallibEntryNumSysTSDGetEnum,
	dallibEntryNumSysTSDSetEnum,
	dallibEntryNumSysThreadInstallExitCallbackEnum,
	dallibEntryNumSysThreadRemoveExitCallbackEnum,
	dallibEntryNumSysCriticalSectionEnterEnum,
	dallibEntryNumSysCriticalSectionExitEnum,
	dallibEntryNumSysConditionVariableWaitEnum,
	dallibEntryNumSysConditionVariableOpenEnum,
	dallibEntryNumSysConditionVariableCloseEnum,
	dallibEntryNumSysConditionVariableBroadcastEnum,
	dallibEntryNumSysOnceFlagTestEnum,
	dallibEntryNumSysOnceFlagSignalEnum,
	dallibEntryNumSysSemaphoreCreateEZEnum,
	dallibEntryNumSysSemaphoreCreateEnum,
	dallibEntryNumSysSemaphoreDestroyEnum,
	dallibEntryNumSysSemaphoreSignalEnum,
	dallibEntryNumSysSemaphoreSignalCountEnum,
	dallibEntryNumSysSemaphoreWaitEnum,
	dallibEntryNumSysSemaphoreWaitCountEnum,
	dallibEntryNumSysProcessIDEnum,
	dallibEntryNumSysProcessNameEnum,
	dallibEntryNumSysThreadGroupCreateEnum,
	dallibEntryNumSysThreadGroupDestroyEnum,
	dallibEntryNumSysThreadGroupWaitEnum,
	dallibEntryNumSysThreadCreateEZEnum,
	dallibEntryNumSysThreadCreateEnum,
	dallibEntryNumSysThreadStartEnum,
	dallibEntryNumSysCurrentThreadEnum,
	dallibEntryNumSysThreadExitEnum,
	dallibEntryNumSysThreadDelayEnum,
	dallibEntryNumSysThreadSuspendEnum,
	dallibEntryNumSysThreadResumeEnum,
	dallibEntryNumSysThreadChangePriorityEnum,
	dallibEntryNumSysThreadSetLibCFuncsEnum,
	dallibEntryNumSysThreadGetLibCFuncsEnum,
	dallibEntryNumKALGetInfoEnum,
	dallibEntryNumKALGetTimeEnum,
	dallibEntryNumKALThreadCreateEnum,
	dallibEntryNumKALThreadDeleteEnum,
	dallibEntryNumKALThreadDestroyEnum,
	dallibEntryNumKALThreadGetInfoEnum,
	dallibEntryNumKALThreadStartEnum,
	dallibEntryNumKALThreadTerminateEnum,
	dallibEntryNumKALThreadUnblockEnum,
	dallibEntryNumKALThreadClearFaultEnum,
	dallibEntryNumKALThreadReplaceKeeperKeyEnum,
	dallibEntryNumKALThreadReplaceSchedulerKeyEnum,
	dallibEntryNumKALThreadSuspendEnum,
	dallibEntryNumKALThreadResumeEnum,
	dallibEntryNumKALThreadForcedResumeEnum,
	dallibEntryNumKALThreadWakeupEnum,
	dallibEntryNumKALThreadCancelWakeupEnum,
	dallibEntryNumKALThreadGetSchedulerKeyEnum,
	dallibEntryNumKALThreadCreateRequestKeyEnum,
	dallibEntryNumKALThreadDestroyRequestKeyEnum,
	dallibEntryNumKALThreadMakeMessageFlagKeyEnum,
	dallibEntryNumKALThreadChangePriorityEnum,
	dallibEntryNumKALThreadIdentifyKeyEnum,
	dallibEntryNumKALThreadReadRegisterEnum,
	dallibEntryNumKALThreadWriteRegisterEnum,
	dallibEntryNumKALThreadReadRegisterSetEnum,
	dallibEntryNumKALThreadWriteRegisterSetEnum,
	dallibEntryNumKALMsgFlagSetEnum,
	dallibEntryNumKALCurrentThreadExitEnum,
	dallibEntryNumKALCurrentThreadExitAndDeleteEnum,
	dallibEntryNumKALCurrentThreadDelayEnum,
	dallibEntryNumKALCurrentThreadTimedSleepEnum,
	dallibEntryNumKALCurrentHardenExitCallbackEnum,
	dallibEntryNumKALSynchronizeCacheEnum,
	dallibEntryNumKALPrivCurrentThreadDeleteEnum,
	dallibEntryNumKALEventFlagCreateEnum,
	dallibEntryNumKALEventFlagDestroyEnum,
	dallibEntryNumKALEventFlagGetInfoEnum,
	dallibEntryNumKALEventFlagClearEnum,
	dallibEntryNumKALEventFlagSetEnum,
	dallibEntryNumKALEventFlagTimedWaitEnum,
	dallibEntryNumKALSemaphoreCreateEnum,
	dallibEntryNumKALSemaphoreDestroyEnum,
	dallibEntryNumKALSemaphoreGetInfoEnum,
	dallibEntryNumKALSemaphoreSignalEnum,
	dallibEntryNumKALSemaphoreSignalCountEnum,
	dallibEntryNumKALSemaphoreTimedWaitEnum,
	dallibEntryNumKALSemaphoreTimedWaitCountEnum,
	dallibEntryNumKrnFastSemaphoreInitEnum,
	dallibEntryNumKALFastSemaphoreCreateEnum,
	dallibEntryNumKALFastSemaphoreDestroyEnum,
	dallibEntryNumKALFastSemaphoreGetInfoEnum,
	dallibEntryNumKALFastSemaphoreSignalEnum,
	dallibEntryNumKALFastSemaphoreWaitEnum,
	dallibEntryNumKALMutexCreateEnum,
	dallibEntryNumKALMutexDestroyEnum,
	dallibEntryNumKALMutexGetInfoEnum,
	dallibEntryNumKALMutexTimedLockEnum,
	dallibEntryNumKALMutexUnlockEnum,
	dallibEntryNumKALResourceBankCreateEnum,
	dallibEntryNumKALResourceBankDestroyEnum,
	dallibEntryNumKALResourceBankGetInfoEnum,
	dallibEntryNumKALResourceBankResetEnum,
	dallibEntryNumKALResourceBankTransferResourceEnum,
	dallibEntryNumKALSegmentCreateEnum,
	dallibEntryNumKALSegmentDestroyEnum,
	dallibEntryNumKALSegmentGetInfoEnum,
	dallibEntryNumKALSegmentAllocateRAMEnum,
	dallibEntryNumKALSegmentFreeRAMEnum,
	dallibEntryNumKALSegmentMakeReadOnlyKeyEnum,
	dallibEntryNumKALSegmentMakeReadWriteKeyEnum,
	dallibEntryNumKALCopyInEnum,
	dallibEntryNumKALCopyOutEnum,
	dallibEntryNumKALProcessCreateEnum,
	dallibEntryNumKALProcessDestroyEnum,
	dallibEntryNumKALProcessGetInfoEnum,
	dallibEntryNumKALProcessReferenceSegmentEnum,
	dallibEntryNumKALProcessUnreferenceSegmentEnum,
	dallibEntryNumKALProcessGetSegmentInfoEnum,
	dallibEntryNumKALProcessGetSegmentKeyEnum,
	dallibEntryNumKALProcessGetThreadKeyEnum,
	dallibEntryNumKALProcessGetKeyEnum,
	dallibEntryNumKALProcessPutKeyEnum,
	dallibEntryNumKALProcessAllocateKeyVariableEnum,
	dallibEntryNumKALProcessAllocateKeyVariableByKeyIDEnum,
	dallibEntryNumKALProcessFreeKeyVariableEnum,
	dallibEntryNumKALProcessSetCacheAttributesEnum,
	dallibEntryNumKALProcessGetCacheAttributesEnum,
	dallibEntryNumKALProcessSetSchedulingClassEnum,
	dallibEntryNumKALProcessSetDebugLevelEnum,
	dallibEntryNumKALSchedulerKeyGetInfoEnum,
	dallibEntryNumKALSchedulerKeyMakeNewKeyEnum,
	dallibEntryNumKALNodeCreateEnum,
	dallibEntryNumKALNodeDestroyEnum,
	dallibEntryNumKALNodeReplaceKeyEnum,
	dallibEntryNumKALNodeGetKeyEnum,
	dallibEntryNumKALNodeMakeNewKeyEnum,
	dallibEntryNumKALNodeTreeCreateEnum,
	dallibEntryNumKALNodeTreeGetSizeEnum,
	dallibEntryNumKALNodeTreeResizeEnum,
	dallibEntryNumKALNodeTreeDestroyEnum,
	dallibEntryNumKALNodeTreeReplaceKeyEnum,
	dallibEntryNumKALNodeTreeGetKeyEnum,
	dallibEntryNumKALBSPInitEnum,
	dallibEntryNumKALBSPRegisterCallbackEnum,
	dallibEntryNumKALBSPGetCallbacksEnum,
	dallibEntryNumKALBSPMapPhysicalAddressesEnum,
	dallibEntryNumKALBSPLockMemoryEnum,
	dallibEntryNumKALBSPUnlockMemoryEnum,
	dallibEntryNumKALBSPAllocateContiguousRAMEnum,
	dallibEntryNumKALBSPEnterPrivilegedModeAndDisableInterruptsEnum,
	dallibEntryNumKALBSPExitPrivilegedModeAndEnableInterruptsEnum,
	dallibEntryNumKALBSPModifyCacheEnum,
	dallibEntryNumKALBSPResetEnum,
	dallibEntryNumKALBSPDebugOperationEnum,
	dallibEntryNumKALKeyGetCategoryEnum,
	dallibEntryNumKALKeyCompareEnum,
	dallibEntryNumKALMsgSendEnum,
	dallibEntryNumKALMsgReceiveEnum,
	dallibEntryNumKALMsgCallEnum,
	dallibEntryNumKALMsgReplyEnum,
	dallibEntryNumKALMsgReplyAndReceiveEnum,
	dallibEntryNumKALMsgSetInstrumentationEnum,
	dallibEntryNumKALProcessCreateBinderRequestKeyEnum,
	dallibEntryNumKALBinderCreateReferenceEnum,
	dallibEntryNumKALBinderDestroyReferenceEnum,
	dallibEntryNumKALBinderPutReferencesEnum,
	dallibEntryNumKALBinderReceiveEnum,
	dallibEntryNumKALBinderCallEnum,
	dallibEntryNumKALBinderReplyEnum,
	dallibEntryNumKALBinderRequestKeyForReferenceEnum,
	dallibEntryNumKALBinderTimeoutEnum,
	dallibEntryNumKALPrivGetStackPointerEnum,
	dallibEntryNumKALPrivStackRepotEnum,
	dallibEntryNumKALPrivCurrentThreadBreakEnum,
	dallibEntryNumKALPrivThreadBreakEnum,
	dallibEntryNumSysAtomicInc32Enum,
	dallibEntryNumSysAtomicDec32Enum,
	dallibEntryNumKALSemaphoreResetEnum,
	dallibEntryNumKALPortCreateEnum,
	dallibEntryNumKALPortDestroyEnum,
	dallibEntryNumKALPortGetInfoEnum,
	dallibEntryNumKALPortCreateRequestKeyEnum,
	dallibEntryNumKALPortDestroyRequestKeyEnum,
	dallibEntryNumKALPortMsgReceiveEnum,
	dallibEntryNumKALBSPGetThreadWatermarkEnum,
	dallibEntryNumKALBSPSetThreadWatermarkEnum,
	dallibEntryNumKALBSPMirrorROMEnum,
};

#define dallibEntryNumRALLinkerStub                                  	((uint32_t)dallibEntryNumRALLinkerStubEnum)
#define dallibEntryNumRALInitNewProcess                              	((uint32_t)dallibEntryNumRALInitNewProcessEnum)
#define dallibEntryNumRALGetModuleInfo                               	((uint32_t)dallibEntryNumRALGetModuleInfoEnum)
#define dallibEntryNumRALLoadModuleWithRPC                           	((uint32_t)dallibEntryNumRALLoadModuleWithRPCEnum)
#define dallibEntryNumRALUnloadModuleWithRPC                         	((uint32_t)dallibEntryNumRALUnloadModuleWithRPCEnum)
#define dallibEntryNumRALLoadBootModule                              	((uint32_t)dallibEntryNumRALLoadBootModuleEnum)
#define dallibEntryNumRALGetEntryAddresses                           	((uint32_t)dallibEntryNumRALGetEntryAddressesEnum)
#define dallibEntryNumRALGetROMHeader                                	((uint32_t)dallibEntryNumRALGetROMHeaderEnum)
#define dallibEntryNumRALGetNVParams                                 	((uint32_t)dallibEntryNumRALGetNVParamsEnum)
#define dallibEntryNumRALGetVectorTable                              	((uint32_t)dallibEntryNumRALGetVectorTableEnum)
#define dallibEntryNumRALRegisterPatch                               	((uint32_t)dallibEntryNumRALRegisterPatchEnum)
#define dallibEntryNumRALLibPreDmInit                                	((uint32_t)dallibEntryNumRALLibPreDmInitEnum)
#define dallibEntryNumRALLibPostDmInit                               	((uint32_t)dallibEntryNumRALLibPostDmInitEnum)
#define dallibEntryNumRALFixupModule                                 	((uint32_t)dallibEntryNumRALFixupModuleEnum)
#define dallibEntryNumRALReloc                                       	((uint32_t)dallibEntryNumRALRelocEnum)
#define dallibEntryNumRALCheckModule                                 	((uint32_t)dallibEntryNumRALCheckModuleEnum)
#define dallibEntryNumRALOnModuleLoaded                              	((uint32_t)dallibEntryNumRALOnModuleLoadedEnum)
#define dallibEntryNumRALOnModuleUnloaded                            	((uint32_t)dallibEntryNumRALOnModuleUnloadedEnum)
#define dallibEntryNumRALIsModuleLoaded                              	((uint32_t)dallibEntryNumRALIsModuleLoadedEnum)
#define dallibEntryNumRALGetLoadedModuleInfo                         	((uint32_t)dallibEntryNumRALGetLoadedModuleInfoEnum)
#define dallibEntryNumRALSetModuleDatabase                           	((uint32_t)dallibEntryNumRALSetModuleDatabaseEnum)
#define dallibEntryNumRALGetModuleGlobals                            	((uint32_t)dallibEntryNumRALGetModuleGlobalsEnum)
#define dallibEntryNumRALBroadcastActionCode                         	((uint32_t)dallibEntryNumRALBroadcastActionCodeEnum)
#define dallibEntryNumRALUnloadUnloadable                            	((uint32_t)dallibEntryNumRALUnloadUnloadableEnum)
#define dallibEntryNumRALLockBootResources                           	((uint32_t)dallibEntryNumRALLockBootResourcesEnum)
#define dallibEntryNumRALCheckBootModule                             	((uint32_t)dallibEntryNumRALCheckBootModuleEnum)
#define dallibEntryNumRALLibOnProcessDestroyed                       	((uint32_t)dallibEntryNumRALLibOnProcessDestroyedEnum)
#define dallibEntryNum_RALInstallPostLoadHook                        	((uint32_t)dallibEntryNum_RALInstallPostLoadHookEnum)
#define dallibEntryNumErrSetJump                                     	((uint32_t)dallibEntryNumErrSetJumpEnum)
#define dallibEntryNumErrLongJump                                    	((uint32_t)dallibEntryNumErrLongJumpEnum)
#define dallibEntryNumErrExceptionListAppend                         	((uint32_t)dallibEntryNumErrExceptionListAppendEnum)
#define dallibEntryNumErrExceptionListRemove                         	((uint32_t)dallibEntryNumErrExceptionListRemoveEnum)
#define dallibEntryNumErrThrowWithAddress                            	((uint32_t)dallibEntryNumErrThrowWithAddressEnum)
#define dallibEntryNumErrThrowWithHandler                            	((uint32_t)dallibEntryNumErrThrowWithHandlerEnum)
#define dallibEntryNumErrExceptionListGetByThreadID                  	((uint32_t)dallibEntryNumErrExceptionListGetByThreadIDEnum)
#define dallibEntryNumDrvMessage                                     	((uint32_t)dallibEntryNumDrvMessageEnum)
#define dallibEntryNumDrvVPrintf                                     	((uint32_t)dallibEntryNumDrvVPrintfEnum)
#define dallibEntryNumDrvPrintf                                      	((uint32_t)dallibEntryNumDrvPrintfEnum)
#define dallibEntryNumDrvBreak                                       	((uint32_t)dallibEntryNumDrvBreakEnum)
#define dallibEntryNumDrvGetChar                                     	((uint32_t)dallibEntryNumDrvGetCharEnum)
#define dallibEntryNumDrvIsPresent                                   	((uint32_t)dallibEntryNumDrvIsPresentEnum)
#define dallibEntryNumDrvOutputSync                                  	((uint32_t)dallibEntryNumDrvOutputSyncEnum)
#define dallibEntryNumDrvFatalErrorInContext                         	((uint32_t)dallibEntryNumDrvFatalErrorInContextEnum)
#define dallibEntryNumSysAtomicAdd32                                 	((uint32_t)dallibEntryNumSysAtomicAdd32Enum)
#define dallibEntryNumSysAtomicAnd32                                 	((uint32_t)dallibEntryNumSysAtomicAnd32Enum)
#define dallibEntryNumSysAtomicOr32                                  	((uint32_t)dallibEntryNumSysAtomicOr32Enum)
#define dallibEntryNumSysAtomicCompareAndSwap32                      	((uint32_t)dallibEntryNumSysAtomicCompareAndSwap32Enum)
#define dallibEntryNumSysAtomicAdd64                                 	((uint32_t)dallibEntryNumSysAtomicAdd64Enum)
#define dallibEntryNumSysAtomicCompareAndSwap64                      	((uint32_t)dallibEntryNumSysAtomicCompareAndSwap64Enum)
#define dallibEntryNumSysGetRunTime                                  	((uint32_t)dallibEntryNumSysGetRunTimeEnum)
#define dallibEntryNumSysTSDAllocate                                 	((uint32_t)dallibEntryNumSysTSDAllocateEnum)
#define dallibEntryNumSysTSDFree                                     	((uint32_t)dallibEntryNumSysTSDFreeEnum)
#define dallibEntryNumSysTSDGet                                      	((uint32_t)dallibEntryNumSysTSDGetEnum)
#define dallibEntryNumSysTSDSet                                      	((uint32_t)dallibEntryNumSysTSDSetEnum)
#define dallibEntryNumSysThreadInstallExitCallback                   	((uint32_t)dallibEntryNumSysThreadInstallExitCallbackEnum)
#define dallibEntryNumSysThreadRemoveExitCallback                    	((uint32_t)dallibEntryNumSysThreadRemoveExitCallbackEnum)
#define dallibEntryNumSysCriticalSectionEnter                        	((uint32_t)dallibEntryNumSysCriticalSectionEnterEnum)
#define dallibEntryNumSysCriticalSectionExit                         	((uint32_t)dallibEntryNumSysCriticalSectionExitEnum)
#define dallibEntryNumSysConditionVariableWait                       	((uint32_t)dallibEntryNumSysConditionVariableWaitEnum)
#define dallibEntryNumSysConditionVariableOpen                       	((uint32_t)dallibEntryNumSysConditionVariableOpenEnum)
#define dallibEntryNumSysConditionVariableClose                      	((uint32_t)dallibEntryNumSysConditionVariableCloseEnum)
#define dallibEntryNumSysConditionVariableBroadcast                  	((uint32_t)dallibEntryNumSysConditionVariableBroadcastEnum)
#define dallibEntryNumSysOnceFlagTest                                	((uint32_t)dallibEntryNumSysOnceFlagTestEnum)
#define dallibEntryNumSysOnceFlagSignal                              	((uint32_t)dallibEntryNumSysOnceFlagSignalEnum)
#define dallibEntryNumSysSemaphoreCreateEZ                           	((uint32_t)dallibEntryNumSysSemaphoreCreateEZEnum)
#define dallibEntryNumSysSemaphoreCreate                             	((uint32_t)dallibEntryNumSysSemaphoreCreateEnum)
#define dallibEntryNumSysSemaphoreDestroy                            	((uint32_t)dallibEntryNumSysSemaphoreDestroyEnum)
#define dallibEntryNumSysSemaphoreSignal                             	((uint32_t)dallibEntryNumSysSemaphoreSignalEnum)
#define dallibEntryNumSysSemaphoreSignalCount                        	((uint32_t)dallibEntryNumSysSemaphoreSignalCountEnum)
#define dallibEntryNumSysSemaphoreWait                               	((uint32_t)dallibEntryNumSysSemaphoreWaitEnum)
#define dallibEntryNumSysSemaphoreWaitCount                          	((uint32_t)dallibEntryNumSysSemaphoreWaitCountEnum)
#define dallibEntryNumSysProcessID                                   	((uint32_t)dallibEntryNumSysProcessIDEnum)
#define dallibEntryNumSysProcessName                                 	((uint32_t)dallibEntryNumSysProcessNameEnum)
#define dallibEntryNumSysThreadGroupCreate                           	((uint32_t)dallibEntryNumSysThreadGroupCreateEnum)
#define dallibEntryNumSysThreadGroupDestroy                          	((uint32_t)dallibEntryNumSysThreadGroupDestroyEnum)
#define dallibEntryNumSysThreadGroupWait                             	((uint32_t)dallibEntryNumSysThreadGroupWaitEnum)
#define dallibEntryNumSysThreadCreateEZ                              	((uint32_t)dallibEntryNumSysThreadCreateEZEnum)
#define dallibEntryNumSysThreadCreate                                	((uint32_t)dallibEntryNumSysThreadCreateEnum)
#define dallibEntryNumSysThreadStart                                 	((uint32_t)dallibEntryNumSysThreadStartEnum)
#define dallibEntryNumSysCurrentThread                               	((uint32_t)dallibEntryNumSysCurrentThreadEnum)
#define dallibEntryNumSysThreadExit                                  	((uint32_t)dallibEntryNumSysThreadExitEnum)
#define dallibEntryNumSysThreadDelay                                 	((uint32_t)dallibEntryNumSysThreadDelayEnum)
#define dallibEntryNumSysThreadSuspend                               	((uint32_t)dallibEntryNumSysThreadSuspendEnum)
#define dallibEntryNumSysThreadResume                                	((uint32_t)dallibEntryNumSysThreadResumeEnum)
#define dallibEntryNumSysThreadChangePriority                        	((uint32_t)dallibEntryNumSysThreadChangePriorityEnum)
#define dallibEntryNumSysThreadSetLibCFuncs                          	((uint32_t)dallibEntryNumSysThreadSetLibCFuncsEnum)
#define dallibEntryNumSysThreadGetLibCFuncs                          	((uint32_t)dallibEntryNumSysThreadGetLibCFuncsEnum)
#define dallibEntryNumKALGetInfo                                     	((uint32_t)dallibEntryNumKALGetInfoEnum)
#define dallibEntryNumKALGetTime                                     	((uint32_t)dallibEntryNumKALGetTimeEnum)
#define dallibEntryNumKALThreadCreate                                	((uint32_t)dallibEntryNumKALThreadCreateEnum)
#define dallibEntryNumKALThreadDelete                                	((uint32_t)dallibEntryNumKALThreadDeleteEnum)
#define dallibEntryNumKALThreadDestroy                               	((uint32_t)dallibEntryNumKALThreadDestroyEnum)
#define dallibEntryNumKALThreadGetInfo                               	((uint32_t)dallibEntryNumKALThreadGetInfoEnum)
#define dallibEntryNumKALThreadStart                                 	((uint32_t)dallibEntryNumKALThreadStartEnum)
#define dallibEntryNumKALThreadTerminate                             	((uint32_t)dallibEntryNumKALThreadTerminateEnum)
#define dallibEntryNumKALThreadUnblock                               	((uint32_t)dallibEntryNumKALThreadUnblockEnum)
#define dallibEntryNumKALThreadClearFault                            	((uint32_t)dallibEntryNumKALThreadClearFaultEnum)
#define dallibEntryNumKALThreadReplaceKeeperKey                      	((uint32_t)dallibEntryNumKALThreadReplaceKeeperKeyEnum)
#define dallibEntryNumKALThreadReplaceSchedulerKey                   	((uint32_t)dallibEntryNumKALThreadReplaceSchedulerKeyEnum)
#define dallibEntryNumKALThreadSuspend                               	((uint32_t)dallibEntryNumKALThreadSuspendEnum)
#define dallibEntryNumKALThreadResume                                	((uint32_t)dallibEntryNumKALThreadResumeEnum)
#define dallibEntryNumKALThreadForcedResume                          	((uint32_t)dallibEntryNumKALThreadForcedResumeEnum)
#define dallibEntryNumKALThreadWakeup                                	((uint32_t)dallibEntryNumKALThreadWakeupEnum)
#define dallibEntryNumKALThreadCancelWakeup                          	((uint32_t)dallibEntryNumKALThreadCancelWakeupEnum)
#define dallibEntryNumKALThreadGetSchedulerKey                       	((uint32_t)dallibEntryNumKALThreadGetSchedulerKeyEnum)
#define dallibEntryNumKALThreadCreateRequestKey                      	((uint32_t)dallibEntryNumKALThreadCreateRequestKeyEnum)
#define dallibEntryNumKALThreadDestroyRequestKey                     	((uint32_t)dallibEntryNumKALThreadDestroyRequestKeyEnum)
#define dallibEntryNumKALThreadMakeMessageFlagKey                    	((uint32_t)dallibEntryNumKALThreadMakeMessageFlagKeyEnum)
#define dallibEntryNumKALThreadChangePriority                        	((uint32_t)dallibEntryNumKALThreadChangePriorityEnum)
#define dallibEntryNumKALThreadIdentifyKey                           	((uint32_t)dallibEntryNumKALThreadIdentifyKeyEnum)
#define dallibEntryNumKALThreadReadRegister                          	((uint32_t)dallibEntryNumKALThreadReadRegisterEnum)
#define dallibEntryNumKALThreadWriteRegister                         	((uint32_t)dallibEntryNumKALThreadWriteRegisterEnum)
#define dallibEntryNumKALThreadReadRegisterSet                       	((uint32_t)dallibEntryNumKALThreadReadRegisterSetEnum)
#define dallibEntryNumKALThreadWriteRegisterSet                      	((uint32_t)dallibEntryNumKALThreadWriteRegisterSetEnum)
#define dallibEntryNumKALMsgFlagSet                                  	((uint32_t)dallibEntryNumKALMsgFlagSetEnum)
#define dallibEntryNumKALCurrentThreadExit                           	((uint32_t)dallibEntryNumKALCurrentThreadExitEnum)
#define dallibEntryNumKALCurrentThreadExitAndDelete                  	((uint32_t)dallibEntryNumKALCurrentThreadExitAndDeleteEnum)
#define dallibEntryNumKALCurrentThreadDelay                          	((uint32_t)dallibEntryNumKALCurrentThreadDelayEnum)
#define dallibEntryNumKALCurrentThreadTimedSleep                     	((uint32_t)dallibEntryNumKALCurrentThreadTimedSleepEnum)
#define dallibEntryNumKALCurrentHardenExitCallback                   	((uint32_t)dallibEntryNumKALCurrentHardenExitCallbackEnum)
#define dallibEntryNumKALSynchronizeCache                            	((uint32_t)dallibEntryNumKALSynchronizeCacheEnum)
#define dallibEntryNumKALPrivCurrentThreadDelete                     	((uint32_t)dallibEntryNumKALPrivCurrentThreadDeleteEnum)
#define dallibEntryNumKALEventFlagCreate                             	((uint32_t)dallibEntryNumKALEventFlagCreateEnum)
#define dallibEntryNumKALEventFlagDestroy                            	((uint32_t)dallibEntryNumKALEventFlagDestroyEnum)
#define dallibEntryNumKALEventFlagGetInfo                            	((uint32_t)dallibEntryNumKALEventFlagGetInfoEnum)
#define dallibEntryNumKALEventFlagClear                              	((uint32_t)dallibEntryNumKALEventFlagClearEnum)
#define dallibEntryNumKALEventFlagSet                                	((uint32_t)dallibEntryNumKALEventFlagSetEnum)
#define dallibEntryNumKALEventFlagTimedWait                          	((uint32_t)dallibEntryNumKALEventFlagTimedWaitEnum)
#define dallibEntryNumKALSemaphoreCreate                             	((uint32_t)dallibEntryNumKALSemaphoreCreateEnum)
#define dallibEntryNumKALSemaphoreDestroy                            	((uint32_t)dallibEntryNumKALSemaphoreDestroyEnum)
#define dallibEntryNumKALSemaphoreGetInfo                            	((uint32_t)dallibEntryNumKALSemaphoreGetInfoEnum)
#define dallibEntryNumKALSemaphoreSignal                             	((uint32_t)dallibEntryNumKALSemaphoreSignalEnum)
#define dallibEntryNumKALSemaphoreSignalCount                        	((uint32_t)dallibEntryNumKALSemaphoreSignalCountEnum)
#define dallibEntryNumKALSemaphoreTimedWait                          	((uint32_t)dallibEntryNumKALSemaphoreTimedWaitEnum)
#define dallibEntryNumKALSemaphoreTimedWaitCount                     	((uint32_t)dallibEntryNumKALSemaphoreTimedWaitCountEnum)
#define dallibEntryNumKrnFastSemaphoreInit                           	((uint32_t)dallibEntryNumKrnFastSemaphoreInitEnum)
#define dallibEntryNumKALFastSemaphoreCreate                         	((uint32_t)dallibEntryNumKALFastSemaphoreCreateEnum)
#define dallibEntryNumKALFastSemaphoreDestroy                        	((uint32_t)dallibEntryNumKALFastSemaphoreDestroyEnum)
#define dallibEntryNumKALFastSemaphoreGetInfo                        	((uint32_t)dallibEntryNumKALFastSemaphoreGetInfoEnum)
#define dallibEntryNumKALFastSemaphoreSignal                         	((uint32_t)dallibEntryNumKALFastSemaphoreSignalEnum)
#define dallibEntryNumKALFastSemaphoreWait                           	((uint32_t)dallibEntryNumKALFastSemaphoreWaitEnum)
#define dallibEntryNumKALMutexCreate                                 	((uint32_t)dallibEntryNumKALMutexCreateEnum)
#define dallibEntryNumKALMutexDestroy                                	((uint32_t)dallibEntryNumKALMutexDestroyEnum)
#define dallibEntryNumKALMutexGetInfo                                	((uint32_t)dallibEntryNumKALMutexGetInfoEnum)
#define dallibEntryNumKALMutexTimedLock                              	((uint32_t)dallibEntryNumKALMutexTimedLockEnum)
#define dallibEntryNumKALMutexUnlock                                 	((uint32_t)dallibEntryNumKALMutexUnlockEnum)
#define dallibEntryNumKALResourceBankCreate                          	((uint32_t)dallibEntryNumKALResourceBankCreateEnum)
#define dallibEntryNumKALResourceBankDestroy                         	((uint32_t)dallibEntryNumKALResourceBankDestroyEnum)
#define dallibEntryNumKALResourceBankGetInfo                         	((uint32_t)dallibEntryNumKALResourceBankGetInfoEnum)
#define dallibEntryNumKALResourceBankReset                           	((uint32_t)dallibEntryNumKALResourceBankResetEnum)
#define dallibEntryNumKALResourceBankTransferResource                	((uint32_t)dallibEntryNumKALResourceBankTransferResourceEnum)
#define dallibEntryNumKALSegmentCreate                               	((uint32_t)dallibEntryNumKALSegmentCreateEnum)
#define dallibEntryNumKALSegmentDestroy                              	((uint32_t)dallibEntryNumKALSegmentDestroyEnum)
#define dallibEntryNumKALSegmentGetInfo                              	((uint32_t)dallibEntryNumKALSegmentGetInfoEnum)
#define dallibEntryNumKALSegmentAllocateRAM                          	((uint32_t)dallibEntryNumKALSegmentAllocateRAMEnum)
#define dallibEntryNumKALSegmentFreeRAM                              	((uint32_t)dallibEntryNumKALSegmentFreeRAMEnum)
#define dallibEntryNumKALSegmentMakeReadOnlyKey                      	((uint32_t)dallibEntryNumKALSegmentMakeReadOnlyKeyEnum)
#define dallibEntryNumKALSegmentMakeReadWriteKey                     	((uint32_t)dallibEntryNumKALSegmentMakeReadWriteKeyEnum)
#define dallibEntryNumKALCopyIn                                      	((uint32_t)dallibEntryNumKALCopyInEnum)
#define dallibEntryNumKALCopyOut                                     	((uint32_t)dallibEntryNumKALCopyOutEnum)
#define dallibEntryNumKALProcessCreate                               	((uint32_t)dallibEntryNumKALProcessCreateEnum)
#define dallibEntryNumKALProcessDestroy                              	((uint32_t)dallibEntryNumKALProcessDestroyEnum)
#define dallibEntryNumKALProcessGetInfo                              	((uint32_t)dallibEntryNumKALProcessGetInfoEnum)
#define dallibEntryNumKALProcessReferenceSegment                     	((uint32_t)dallibEntryNumKALProcessReferenceSegmentEnum)
#define dallibEntryNumKALProcessUnreferenceSegment                   	((uint32_t)dallibEntryNumKALProcessUnreferenceSegmentEnum)
#define dallibEntryNumKALProcessGetSegmentInfo                       	((uint32_t)dallibEntryNumKALProcessGetSegmentInfoEnum)
#define dallibEntryNumKALProcessGetSegmentKey                        	((uint32_t)dallibEntryNumKALProcessGetSegmentKeyEnum)
#define dallibEntryNumKALProcessGetThreadKey                         	((uint32_t)dallibEntryNumKALProcessGetThreadKeyEnum)
#define dallibEntryNumKALProcessGetKey                               	((uint32_t)dallibEntryNumKALProcessGetKeyEnum)
#define dallibEntryNumKALProcessPutKey                               	((uint32_t)dallibEntryNumKALProcessPutKeyEnum)
#define dallibEntryNumKALProcessAllocateKeyVariable                  	((uint32_t)dallibEntryNumKALProcessAllocateKeyVariableEnum)
#define dallibEntryNumKALProcessAllocateKeyVariableByKeyID           	((uint32_t)dallibEntryNumKALProcessAllocateKeyVariableByKeyIDEnum)
#define dallibEntryNumKALProcessFreeKeyVariable                      	((uint32_t)dallibEntryNumKALProcessFreeKeyVariableEnum)
#define dallibEntryNumKALProcessSetCacheAttributes                   	((uint32_t)dallibEntryNumKALProcessSetCacheAttributesEnum)
#define dallibEntryNumKALProcessGetCacheAttributes                   	((uint32_t)dallibEntryNumKALProcessGetCacheAttributesEnum)
#define dallibEntryNumKALProcessSetSchedulingClass                   	((uint32_t)dallibEntryNumKALProcessSetSchedulingClassEnum)
#define dallibEntryNumKALProcessSetDebugLevel                        	((uint32_t)dallibEntryNumKALProcessSetDebugLevelEnum)
#define dallibEntryNumKALSchedulerKeyGetInfo                         	((uint32_t)dallibEntryNumKALSchedulerKeyGetInfoEnum)
#define dallibEntryNumKALSchedulerKeyMakeNewKey                      	((uint32_t)dallibEntryNumKALSchedulerKeyMakeNewKeyEnum)
#define dallibEntryNumKALNodeCreate                                  	((uint32_t)dallibEntryNumKALNodeCreateEnum)
#define dallibEntryNumKALNodeDestroy                                 	((uint32_t)dallibEntryNumKALNodeDestroyEnum)
#define dallibEntryNumKALNodeReplaceKey                              	((uint32_t)dallibEntryNumKALNodeReplaceKeyEnum)
#define dallibEntryNumKALNodeGetKey                                  	((uint32_t)dallibEntryNumKALNodeGetKeyEnum)
#define dallibEntryNumKALNodeMakeNewKey                              	((uint32_t)dallibEntryNumKALNodeMakeNewKeyEnum)
#define dallibEntryNumKALNodeTreeCreate                              	((uint32_t)dallibEntryNumKALNodeTreeCreateEnum)
#define dallibEntryNumKALNodeTreeGetSize                             	((uint32_t)dallibEntryNumKALNodeTreeGetSizeEnum)
#define dallibEntryNumKALNodeTreeResize                              	((uint32_t)dallibEntryNumKALNodeTreeResizeEnum)
#define dallibEntryNumKALNodeTreeDestroy                             	((uint32_t)dallibEntryNumKALNodeTreeDestroyEnum)
#define dallibEntryNumKALNodeTreeReplaceKey                          	((uint32_t)dallibEntryNumKALNodeTreeReplaceKeyEnum)
#define dallibEntryNumKALNodeTreeGetKey                              	((uint32_t)dallibEntryNumKALNodeTreeGetKeyEnum)
#define dallibEntryNumKALBSPInit                                     	((uint32_t)dallibEntryNumKALBSPInitEnum)
#define dallibEntryNumKALBSPRegisterCallback                         	((uint32_t)dallibEntryNumKALBSPRegisterCallbackEnum)
#define dallibEntryNumKALBSPGetCallbacks                             	((uint32_t)dallibEntryNumKALBSPGetCallbacksEnum)
#define dallibEntryNumKALBSPMapPhysicalAddresses                     	((uint32_t)dallibEntryNumKALBSPMapPhysicalAddressesEnum)
#define dallibEntryNumKALBSPLockMemory                               	((uint32_t)dallibEntryNumKALBSPLockMemoryEnum)
#define dallibEntryNumKALBSPUnlockMemory                             	((uint32_t)dallibEntryNumKALBSPUnlockMemoryEnum)
#define dallibEntryNumKALBSPAllocateContiguousRAM                    	((uint32_t)dallibEntryNumKALBSPAllocateContiguousRAMEnum)
#define dallibEntryNumKALBSPEnterPrivilegedModeAndDisableInterrupts  	((uint32_t)dallibEntryNumKALBSPEnterPrivilegedModeAndDisableInterruptsEnum)
#define dallibEntryNumKALBSPExitPrivilegedModeAndEnableInterrupts    	((uint32_t)dallibEntryNumKALBSPExitPrivilegedModeAndEnableInterruptsEnum)
#define dallibEntryNumKALBSPModifyCache                              	((uint32_t)dallibEntryNumKALBSPModifyCacheEnum)
#define dallibEntryNumKALBSPReset                                    	((uint32_t)dallibEntryNumKALBSPResetEnum)
#define dallibEntryNumKALBSPDebugOperation                           	((uint32_t)dallibEntryNumKALBSPDebugOperationEnum)
#define dallibEntryNumKALKeyGetCategory                              	((uint32_t)dallibEntryNumKALKeyGetCategoryEnum)
#define dallibEntryNumKALKeyCompare                                  	((uint32_t)dallibEntryNumKALKeyCompareEnum)
#define dallibEntryNumKALMsgSend                                     	((uint32_t)dallibEntryNumKALMsgSendEnum)
#define dallibEntryNumKALMsgReceive                                  	((uint32_t)dallibEntryNumKALMsgReceiveEnum)
#define dallibEntryNumKALMsgCall                                     	((uint32_t)dallibEntryNumKALMsgCallEnum)
#define dallibEntryNumKALMsgReply                                    	((uint32_t)dallibEntryNumKALMsgReplyEnum)
#define dallibEntryNumKALMsgReplyAndReceive                          	((uint32_t)dallibEntryNumKALMsgReplyAndReceiveEnum)
#define dallibEntryNumKALMsgSetInstrumentation                       	((uint32_t)dallibEntryNumKALMsgSetInstrumentationEnum)
#define dallibEntryNumKALProcessCreateBinderRequestKey               	((uint32_t)dallibEntryNumKALProcessCreateBinderRequestKeyEnum)
#define dallibEntryNumKALBinderCreateReference                       	((uint32_t)dallibEntryNumKALBinderCreateReferenceEnum)
#define dallibEntryNumKALBinderDestroyReference                      	((uint32_t)dallibEntryNumKALBinderDestroyReferenceEnum)
#define dallibEntryNumKALBinderPutReferences                         	((uint32_t)dallibEntryNumKALBinderPutReferencesEnum)
#define dallibEntryNumKALBinderReceive                               	((uint32_t)dallibEntryNumKALBinderReceiveEnum)
#define dallibEntryNumKALBinderCall                                  	((uint32_t)dallibEntryNumKALBinderCallEnum)
#define dallibEntryNumKALBinderReply                                 	((uint32_t)dallibEntryNumKALBinderReplyEnum)
#define dallibEntryNumKALBinderRequestKeyForReference                	((uint32_t)dallibEntryNumKALBinderRequestKeyForReferenceEnum)
#define dallibEntryNumKALBinderTimeout                               	((uint32_t)dallibEntryNumKALBinderTimeoutEnum)
#define dallibEntryNumKALPrivGetStackPointer                         	((uint32_t)dallibEntryNumKALPrivGetStackPointerEnum)
#define dallibEntryNumKALPrivStackRepot                              	((uint32_t)dallibEntryNumKALPrivStackRepotEnum)
#define dallibEntryNumKALPrivCurrentThreadBreak                      	((uint32_t)dallibEntryNumKALPrivCurrentThreadBreakEnum)
#define dallibEntryNumKALPrivThreadBreak                             	((uint32_t)dallibEntryNumKALPrivThreadBreakEnum)
#define dallibEntryNumSysAtomicInc32                                 	((uint32_t)dallibEntryNumSysAtomicInc32Enum)
#define dallibEntryNumSysAtomicDec32                                 	((uint32_t)dallibEntryNumSysAtomicDec32Enum)
#define dallibEntryNumKALSemaphoreReset                              	((uint32_t)dallibEntryNumKALSemaphoreResetEnum)
#define dallibEntryNumKALPortCreate                                  	((uint32_t)dallibEntryNumKALPortCreateEnum)
#define dallibEntryNumKALPortDestroy                                 	((uint32_t)dallibEntryNumKALPortDestroyEnum)
#define dallibEntryNumKALPortGetInfo                                 	((uint32_t)dallibEntryNumKALPortGetInfoEnum)
#define dallibEntryNumKALPortCreateRequestKey                        	((uint32_t)dallibEntryNumKALPortCreateRequestKeyEnum)
#define dallibEntryNumKALPortDestroyRequestKey                       	((uint32_t)dallibEntryNumKALPortDestroyRequestKeyEnum)
#define dallibEntryNumKALPortMsgReceive                              	((uint32_t)dallibEntryNumKALPortMsgReceiveEnum)
#define dallibEntryNumKALBSPGetThreadWatermark                       	((uint32_t)dallibEntryNumKALBSPGetThreadWatermarkEnum)
#define dallibEntryNumKALBSPSetThreadWatermark                       	((uint32_t)dallibEntryNumKALBSPSetThreadWatermarkEnum)
#define dallibEntryNumKALBSPMirrorROM                                	((uint32_t)dallibEntryNumKALBSPMirrorROMEnum)
#endif // __DALLIBENTRYNUMS_H__
