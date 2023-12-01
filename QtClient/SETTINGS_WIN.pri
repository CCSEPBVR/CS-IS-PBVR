CONFIG -= debug_and_release debug_and_release_target
QMAKE_CFLAGS_RELEASE += /MT
QMAKE_CXXFLAGS_RELEASE += /MT
QMAKE_CFLAGS_DEBUG += /MTd
QMAKE_CXXFLAGS_DEBUG += /MTd
#QMAKE_CXXFLAGS_RELEASE += /source-charset:utf-8
#QMAKE_CXXFLAGS_DEBUG += /source-charset:utf-8
QMAKE_CXXFLAGS_RELEASE += /Wall

# disable warnings
QMAKE_CXXFLAGS_RELEASE += /wd4710 #inline function
QMAKE_CXXFLAGS_RELEASE += /wd4711 #inline function
QMAKE_CXXFLAGS_RELEASE += /wd4883 #suppresses optimizations
QMAKE_CXXFLAGS_RELEASE += /wd5204 #destructor
QMAKE_CXXFLAGS_RELEASE += /wd5045 #Qspectre 

DEFINES += WIN32_LEAN_AND_MEAN
DEFINES += NOMINMAX
#PBVR LOCATION
PBVR_PATH    = ../..

#DEPLOY DIRECTORY
DEPLOYDIR = "../deploy_client"

CONFIG(debug, debug|release) :RELTYPE=debug
CONFIG(release, debug|release) :RELTYPE=release
#ADD LINKING FOR PBVR BUILD LOCATION
PBVR_LPATH=$${PBVR_PATH}/x64/$${RELTYPE}

LIB_SUFFIX = .lib
KVS_DIR = ../../QtClient/KVS_build/$${RELTYPE}
FP_DIR = $${PBVR_LPATH}

#FIGURE OUT BUILD DIR
Release:BUILDDIR = $$OUT_PWD/release
Debug:BUILDDIR = $$OUT_PWD/debug

#FIGURE OUT WINDEPLOYQT FLAG
Release:RELDBUG = --release
Debug:RELDBUG = --debug

COMPATH = ../Common
COMPATH2 = $${PBVR_PATH}/Common


#message( INCLUDING FOR PLATFORM $${PLATFORM} $${RELTYPE}. PBVR_LPATH: $${PBVR_LPATH} LIB:$${LIBS} )
