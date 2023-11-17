/***************************************************************
 *
 * Project:
 *    ViewerUtils 
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    StringUtils.c
 * 
 * Description:
 *    This file contains utility functions dealing with strings
 *    and string resources. 
 *
 * ToDo:
 *
 * History:
 *    11-aug-1999 dia  Created by Douglas Anderson (dianders@handspring.com)
 *    17-dec-1999 dia  Made OS 3.5 header friendly (no functional changes)
 *	  11-jul-2000 dia  For the space conscious, separated out each function
 *					   into an individual file until we get a compiler that
 *					   is smart enough to realize that unused functions don't
 *					   need to be included (temporary workaround).
 *
 ****************************************************************/

#include <stdarg.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <TextMgr.h>

#include "StringUtils.h"

#include "StringUtils.GetStringResource.c"
#include "StringUtils.StrVariableSubst.c"
#include "StringUtils.StrSpn.c"
#include "StringUtils.StrCSpn.c"
#include "StringUtils.StrDup.c"
#include "StringUtils.StrNCopyZ.c"
#include "StringUtils.StrTruncateToW.c"
#include "StringUtils.HsUtilWinDrawTCWJ.c"