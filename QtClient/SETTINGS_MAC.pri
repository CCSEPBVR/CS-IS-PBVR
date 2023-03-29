CONFIG -= debug_and_release debug_and_release_target

#DEFINES += WIN32_LEAN_AND_MEAN
DEFINES += NOMINMAX
#PBVR LOCATION
PBVR_PATH    = ../..

#DEPLOY DIRECTORY
DEPLOYDIR = "../deploy_client"

#ADD LINKING FOR PBVR BUILD LOCATION
PBVR_LPATH=$${PBVR_PATH}/x64/$${RELTYPE}

LIB_PREFIX = lib
LIB_SUFFIX = .a


KVS_DIR = ../../QtClient/KVS/
FP_DIR  = ../../FunctionParser

#FIGURE OUT BUILD DIR
Release:BUILDDIR = $$OUT_PWD/release
Debug:BUILDDIR = $$OUT_PWD/debug

#FIGURE OUT WINDEPLOYQT FLAG
Release:RELDBUG = --release
Debug:RELDBUG = --debug

COMPATH = ../Common
COMPATH2 = $${PBVR_PATH}/Common

message( INCLUDING FOR PLATFORM $${PLATFORM} $${RELTYPE}. PBVR_LPATH: $${PBVR_LPATH} LIB:$${LIBS} )

#DEPLOY DIRECTORY
debug:DEPLOYDIR = "../deploy_client_debug"
release:DEPLOYDIR = "../deploy_client"

#ADD LINKING FOR PBVR BUILD LOCATION
debug:PBVR_LPATH=$${PBVR_PATH}/
release:PBVR_LPATH=$${PBVR_PATH}/
