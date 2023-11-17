## --------------------------------------------------------------------------
#  Palm OS Generic Protein Makefile for Eclipse v1.0.0
#
# Fill in this file to specify your project and the source that you want
# to build, and the settings involved in the build. The makefile-engine.mk
# will then do the hard work of the makefile and dependency handling.
#
# After starting a new project, please remember the following steps...
#	1. Add all sources and resources in SOURCES and RESOURCES
#	2. Review the other settings as needed.
#
## --------------------------------------------------------------------------

SHELL = /bin/sh

## conditionally include an auto-generated.mk for dynamic definitions
-include auto-generated.mk

## --------------------------------------------------------------------------
# Set up the artifact name.
# The database name and other file names are based on the application name
## --------------------------------------------------------------------------
ARTIFACT_NAME = RAM_ToDo
EMPTY =
SPACE =$(EMPTY) $(EMPTY)
ESCAPED_ARTIFACT_NAME = $(subst $(SPACE),\ ,$(ARTIFACT_NAME))

PROJECT_NAME = ToDo
PRC_NAME = RAM_ToDo.prc
APP_NAME = RAM_ToDo

## --------------------------------------------------------------------------
# Sources and Resources and Definition files
# List all the sources (.c/.cpp), resources (.xrd), and definition file (.sld)
# in your project.  Use project relative path names with forward slashes 
# (src/code.cpp).  Please do not use spaces in directory or file names.
#
# A note about XRD resource files:  If you have existing .rsrc or .rcp files, 
# refer to the documentation for the GenerateXRD tool to convert them into 
# XRD files for use with all Palm OS SDKs.
## --------------------------------------------------------------------------

# TODO: Update all sources and resources

SOURCES = ToDo.c ToDoDB.c ToDoInDatebook.c ToDoTransfer.c
RESOURCES = rsc/ToDoDefaultDB.xrd rsc/ToDoDelete.xrd rsc/ToDoDeleteComplete.xrd rsc/ToDoDetails.xrd rsc/ToDoList.xrd rsc/ToDoMisc.xrd rsc/ToDoOptions.xrd \
            rsc/localized/enUS/ToDoDefaultDB.xrd rsc/localized/enUS/ToDoDelete.xrd rsc/localized/enUS/ToDoDeleteComplete.xrd rsc/localized/enUS/ToDoDetails.xrd rsc/localized/enUS/ToDoList.xrd rsc/localized/enUS/ToDoMisc.xrd rsc/localized/enUS/ToDoOptions.xrd
SLD_FILE = app.sld
DEFS_FILE = ToDo.def
## --------------------------------------------------------------------------
# Review Database information
# Register Creator ID at: http://dev.palmos.com/creatorid/
## --------------------------------------------------------------------------

CREATOR_ID = ramt
DB_TYPE = appl

DATABASE_RESET =
DATABASE_BACKUP = -b
DATABASE_HIDDEN =
DATABASE_PROTECT =
DATABASE_VERSION = 1

LOCALE = enUS
DATABASE_NAME = $(APP_NAME)

## --------------------------------------------------------------------------
# Choose SDK
# Supported values:
#	sdk-6
## --------------------------------------------------------------------------

SDK_VERSION = sdk-6

ifeq ($(SDK_LOCATION),)
SDK_LOCATION=../../
endif
## --------------------------------------------------------------------------
# Build Settings
# Review the following for your needs.
# The default settings build with debug information and no optimization and
# a target platform of an actual device.
## --------------------------------------------------------------------------

#
# Set Debug or Release configuration for this project via the project 
# properties (right-click on the project and select Properties).
#
DEBUG_OR_RELEASE=Debug

#
# Set the target platform for the build; either Device or Simulator
# Use Device as a target for emulator builds.
#
TARGET_PLATFORM=Simulator

# Specify the level of optimization that you want:
# NONE, SOME, FULL, INTRAPROCEDURAL, INTERPROCEDURAL, INLINING.
# Leave blank to select FULL for release builds, and NONE for
# debug builds.
# INTRAPROCEDURAL, INTERPROCEDURAL, INLINING are applicable to 
# device builds only; simulator builds will translate those levels
# to FULL.
OPTIMIZE_LEVEL = NONE

# Specify warning level
# NONE = suppress all warnings
# ALL = enable all warnings
# <undefined> = default warnings
WARNING_LEVEL = 

# Specify exception handling support
# true = enable it
# false = don't enable it
ENABLE_EXCEPTION_HANDLING=true

# List additional libraries to link with
# (Full or project relative path)
ifeq ($(DEBUG_OR_RELEASE), Debug)
ifeq ($(TARGET_PLATFORM), Device)
ADDITIONAL_LINK_LIBRARIES = ../Libraries/SearchLib/DebugDevice/RAM_SearchLib_Client.sa ../Libraries/NoteViewLib/DebugDevice/RAM_NoteViewLib_Client.sa
else
ADDITIONAL_SIM_LINK_LIBRARIES = ../Libraries/SearchLib/DebugSim/RAM_SearchLib_Client.slib ../Libraries/NoteViewLib/DebugSim/RAM_NoteViewLib_Client.slib
endif
endif

ifeq ($(DEBUG_OR_RELEASE), Release)
ifeq ($(TARGET_PLATFORM), Device)
ADDITIONAL_LINK_LIBRARIES = ../Libraries/SearchLib/ReleaseDevice/RAM_SearchLib_Client.sa ../Libraries/NoteViewLib/ReleaseDevice/RAM_NoteViewLib_Client.sa
else
ADDITIONAL_SIM_LINK_LIBRARIES = ../Libraries/SearchLib/ReleaseSim/RAM_SearchLib_Client.slib ../Libraries/NoteViewLib/ReleaseSim/RAM_NoteViewLib_Client.slib
endif
endif
# Additional paths to look for #include "header"
# (Source file directories are automatically included)
# Please note that both local and system include paths should 
# either use "cygwin" syntax or if in Windows syntax the should
# be quoted.  ie: ../MyLibrary/headers or "C:\Source\MyLibrary\headers"
# Additionally, you must explicly specify the "-I" prior to each
# path included in this variable.  Spaces are used to separate
# each path from each other.
LOCAL_INCLUDE_PATHS = -Irsc -I../Libraries/NoteViewLib/ 
LOCAL_SIM_INCLUDE_PATHS = -Irsc -I../Libraries/NoteViewLib/

# Additional paths to look for #include <header>
# (Palm OS SDK directories are automatically included)
# Additionally, you must explicly specify the "-I" prior to each
# path included in this variable.  Spaces are used to separate
# each path from each other.
SYSTEM_INCLUDE_PATHS = -I../Libraries/SearchLib/
SYSTEM_SIM_INCLUDE_PATHS = -I../Libraries/SearchLib/

# Specify any needed preprocessor symbols.
# If you set DEFINES to "ASSERTLEVEL=1", the compiler will see "-DASSERTLEVEL=1"
# (separate individual entries with spaces)
DEFINES =
SIM_DEFINES =

# Specify additional compiler flags for all files
ADDITIONAL_COMPILER_FLAGS = 
ADDITIONAL_SIM_COMPILER_FLAGS =

# Specify additional linker flags
ADDITIONAL_LINKER_FLAGS =
ADDITIONAL_SIM_LINKER_FLAGS =

# Specify additional archival flags (for static libraries)
ADDITIONAL_AR_FLAGS =
ADDITIONAL_SIM_AR_FLAGS =


# Tools Directory
# Normally you wouldn't want to override this since the necessary tools
# are in the PATH.  But if you experimenting with other tools, then 
# reset the directory here
# If overridden, end with a trailing '/'

ifeq ($(TOOLS_DIR), )
TOOLS_DIR = ../../../PalmOSTools/
endif

## --------------------------------------------------------------------------
# Resource Settings
## --------------------------------------------------------------------------

# Specify the target text encoding
# LATIN, JAPANESE, SIMPLIFIED_CHINESE
TEXTENCODING = LATIN

# Specify error levels for locale and text encoding
# NOCHECK
# STRICT
# <default>
LOCALE_CHECKING =

# Specific if encoding should be checked
# FALSE, TRUE
STRICT_ENCODING = FALSE

## --------------------------------------------------------------------------
# Output Settings
## --------------------------------------------------------------------------

# Modify if you want object and output files to be in different locations
# (However, please note that all three object directories must be different
# for the makefile engine to properly operate.)
# OUTPUT_DIR is the "main" output directory, where all linked and binary objects
# will be put.

DEBUG_DEVICE_OUTPUT_DIR := DebugDevice
RELEASE_DEVICE_OUTPUT_DIR := ReleaseDevice
DEBUG_SIMULATOR_OUTPUT_DIR := DebugSim
RELEASE_SIMULATOR_OUTPUT_DIR := ReleaseSim
RSC_OBJ_DIR := ResourceObjs

## --------------------------------------------------------------------------
# Makefiles
## --------------------------------------------------------------------------


## include the makefile engine
include makefile-engine.mk

## conditionally include your own custom.mk for your specific overrides or definitions
## this is useful if you don't want to use the auto-generated.mk values
## for some reason
-include custom.mk
