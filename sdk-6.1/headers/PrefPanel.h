/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: PrefPanel.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *       Private entry points for preference panels to use
 *
 *****************************************************************************/

#ifndef __PREFPANEL_H__
#define __PREFPANEL_H__

#include <Form.h>
#include <PalmTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


Boolean PrefPanelTitleGadgetHandler(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP);

#ifdef __cplusplus
}
#endif

#endif
