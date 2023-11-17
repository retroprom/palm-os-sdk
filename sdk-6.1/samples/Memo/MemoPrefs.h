/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: MemoPrefs.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef __MEMOPREFS_H__
#define __MEMOPREFS_H__

/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

Boolean PreferencesHandleEvent(EventType *event);

void MemoLoadPrefs(void);
void MemoSavePrefs(uint32_t scrollPosition);

void MemoLoadCurrentCategories(uint32_t *numCategoriesP, CategoryID **categoriesPP);
void MemoSaveCurrentCategories(uint32_t numCategories, CategoryID *categoriesP);

#ifdef __cplusplus
}
#endif

#endif /* __MEMOPREFS_H__ */
