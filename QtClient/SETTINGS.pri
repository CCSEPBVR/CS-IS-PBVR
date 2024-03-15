## VERSION
DEFINES += PBVR_VERSION="1.15-UF"
## DETERMINE PLATFORM  ##
win32:PLATFORM=WIN
unix:!macx:PLATFORM=UNX
macx:PLATFORM=MAC
## DETERMINE RELEASE TYPE ##
CONFIG(debug, debug|release) :RELTYPE=debug
CONFIG(release, debug|release) :RELTYPE=release

## INCLUDE PLATFORM SETTINGS ##
include(SETTINGS_$${PLATFORM}.pri)
include(qtpbvr.conf)
## DETERMINE PBVR_MODE MODE ##

equals(PBVR_MODE, IS): DEFINES += IS_MODE
equals(PBVR_MODE, CS): DEFINES += CS_MODE
equals(PBVR_MODE, VR): DEFINES += VR_MODE
equals(PBVR_MODE, MR): DEFINES += MR_MODE
equals(REND_MODE, CPU): DEFINES += CPU_MODE
equals(REND_MODE, GPU): DEFINES += GPU_MODE

## DETERMINE PBVR_MODE MODE ##
!contains(DEFINES, IS_MODE):!contains(DEFINES, CS_MODE):!contains(DEFINES, VR_MODE):!contains(DEFINES, MR_MODE){
  DEFINES += CS_MODE
  PBVR_MODE = CS
}

contains(DEFINES, IS_MODE) {
  DEFINES += COMM_MODE_IS
} else {
  DEFINES += COMM_MODE_CS
}

contains(DEFINES, VR_MODE)|contains(DEFINES, MR_MODE) {
  DEFINES += DISP_MODE_VR
} else {
  DEFINES += DISP_MODE_2D
}

# enable cg-model library
DEFINES += ENABLE_FEATURE_DRAW_TEXTURED_POLYGON
DEFINES += CGFORMATEXT4KVS_SUPPORT_ASSIMP
DEFINES += ENABLE_ASSIMP
DEFINES += CGFORMATEXT4KVS_SUPPORT_FBXSDK
#DEFINES += ENABLE_FBXSDK
contains(DEFINES, CS_MODE){
DEFINES += ENABLE_LIBLAS
}

DEFINES += ENABLE_INITIAL_XFORM_FROM_PARTICLE_ONLY
DEFINES += ENABLE_LOAD_SHADER_STATIC

DEFINES += ENABLE_FEATURE_DRAW_PARTICLE
DEFINES += DEBUG_ENABLE_DUMP_STACKTRACE

DEFINES += ENABLE_FEATURE_DRAW_BOUNDING_BOX
DEFINES += ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
DEFINES += BOUNDING_BOX_USE_OBJECT_COORD
#DEFINES += BOUNDING_BOX_USE_EXTRENAL_COORD

#DEFINES += ENABLE_TIME_MEASURE

# VR mode or MR mode
contains(DEFINES,VR_MODE)|contains(DEFINES,MR_MODE) {
    ## mirror image mode
    # default : BOTH_DISTORTED
    #DEFINES += MIRROR_IMAGE_BOTH_DISTORTED
    DEFINES += MIRROR_IMAGE_BOTH
    #DEFINES += MIRROR_IMAGE_LEFT_ONLY
    #DEFINES += MIRROR_IMAGE_RIGHT_ONLY

    ## grab button
    # default : hand trigger
    DEFINES += GRAB_USING_HAND_TRIGGER
    #DEFINES += GRAB_USING_INDEX_TRIGGER

    ## BENCHMARK mode
    #DEFINES += ENABLE_BENCHMARK_POLYGON
    #DEFINES += ENABLE_BENCHMARK_PARTICLE

    ### "ENABLE_CGMODEL_EXT_COORD_MODE" flag renamed to "ENABLE_EXTCRD_SCALING_FOR_DEMO"
    ## If this flag is defined, CG polygon and particle objects are forcely transformed to same centerposition and size.
    # When demonstrate PBVR using models with different coordinates/size/positions, enable this flag.
    #DEFINES += ENABLE_EXTCRD_SCALING_FOR_DEMO

    ## MR mode only
    contains(DEFINES, MR_MODE) {
        DEFINES += USE_ZED_MUTEX
        DEFINES += USE_ZED_CAMERA_RENDERING

        #DEFINES += DEBUG_DEPTH_MAP_ON_GPU

        ## for test
        DEFINES += ENABLE_FEATURE_DRAW_CAMERA_IMAGE

        #DEFINES += ENABLE_MTX_LOCK
        #DEFINES += DEBUG_ZED
        #DEFINES += DEBUG_ZED_VP
    }

    ## for test
    DEFINES += ENABLE_FEATURE_DRAW_VR_HANDS
    DEFINES += ENABLE_FEATURE_DRAW_CONTROL_SPHERE

    DEFINES += CHECK_ZOOM_RATIO
    #DEFINES += UPDATE_FRAME_LABELS_ALWAYS

    #DEFINES += CALL_GL_FINISH

    #DEFINES += ENABLE_SNAPSHOT

    ## debug
    ##DEFINES += DEBUG_SCENE
    ##DEFINES += DEBUG_SCREEN
    #DEFINES += DEBUG_TOUCH_CONTROLLER
    #DEFINES += ENABLE_BUGSHOOTING_MODE
    #DEFINES += DEBUG_COMPOSITOR
    #DEFINES += DISABLE_LINES
    #DEFINES += DEBUG_BBOX
}

#DEFINES += CHECK_INITIAL_XFORM

## WARN ON DEPRECEATED QT FUNC ##
DEFINES += QT_DEPRECATED_WARNINGS
## DISABLE DEPRECEATED QT FUNC (disables all the APIs deprecated before Qt 6.0.0) ##
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

## USE PRECOMPILED HEADERS ##
precompile_header:!isEmpty(PRECOMPILED_HEADER) {
DEFINES += USING_PCH
}


# string expression of COMM_MODE_IS or COMM_MODE_CS
COMM_MODE_STR=CS
contains(DEFINES, IS_MODE):COMM_MODE_STR=IS

#PBVR INCLUDE PATHS
INCLUDEPATH += ../
INCLUDEPATH += $$(KVS_DIR)/include
INCLUDEPATH += ../Common/$${COMM_MODE_STR}
INCLUDEPATH += ../FunctionParser
#INCLUDEPATH += $${PBVR_PATH}/FunctionParser
INCLUDEPATH += $$(IMGUI_DIR)

INCLUDEPATH += $$(CGFORMAT_EXT4KVS_SHADER_DIR)
contains(DEFINES, VR_MODE)|contains(DEFINES, MR_MODE) {
    INCLUDEPATH += $$(OCULUS_INC_DIR)

    contains(DEFINES, MR_MODE) {
        INCLUDEPATH += '$$(ZED_SDK_ROOT_DIR)/include'
        INCLUDEPATH += '$$(CUDA_PATH_V11_0)/include'
    }
}

# enable openGL
DEFINES += KVS_ENABLE_OPENGL

KVS_GLEW_DIR = $$(KVS_GLEW_DIR)
message(KVS_GLEW_DIR :: $$(KVS_GLEW_DIR))
!isEmpty(KVS_GLEW_DIR){
    message("KVS_GLEW_DIR in configured. defining KVS_ENABLE_GLEW")
    DEFINES += KVS_ENABLE_GLEW
    INCLUDEPATH += $${KVS_GLEW_DIR}/include
}

contains(DEFINES, ENABLE_LIBLAS)
{
    win32 {
        INCLUDEPATH += $$(Boost_INCLUDE_DIR)
    }
    else {
        INCLUDEPATH += $$(BOOST_INC_DIR)
    }
INCLUDEPATH += $$(LIBLAS_INC_DIR)
}

message("INCLUDE PATH IS::"$${INCLUDEPATH})