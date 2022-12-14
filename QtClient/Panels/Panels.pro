TARGET   = Panels
TEMPLATE = lib
CONFIG += static
#QT       += core gui opengl openglextensions widgets
QT       += core gui opengl

#PRECOMPILED_HEADER=stable.h

include( ../SETTINGS.pri)
message (Building PBVR:$${TARGET} -  Mode:$${PBVR_MODE}  - Platform: $${PLATFORM}  - ReleaseType: $${RELTYPE})

CONFIG += warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

HEADERS += transferfunceditor.h \
    slidercontrol.h
HEADERS += transferfunceditor_$${PBVR_MODE}.h
SOURCES += transferfunceditor_$${PBVR_MODE}.cpp \
    slidercontrol.cpp

HEADERS += \
    coloreditdialog.h \
    filterinfopanel.h \
    legendpanel.h \
    particlepanel.h \
    renderoptionpanel.h \
    timecontrolpanel.h \
    transformpanel.h \
    coordinatepanel.h \
    systemstatuspanel.h \
    coloreditor.h \
    opacityeditor.h \
    animationcontrols.h \
    functionlisteditor.h \
    viewercontrols.h


SOURCES += \
    coloreditdialog.cpp \
    filterinfopanel.cpp \
    legendpanel.cpp \
    particlepanel.cpp \
    renderoptionpanel.cpp \
    timecontrolpanel.cpp \
    transformpanel.cpp \
    coordinatepanel.cpp \
    systemstatuspanel.cpp \
    coloreditor.cpp \
    opacityeditor.cpp \
    animationcontrols.cpp \
    functionlisteditor.cpp \
    viewercontrols.cpp

FORMS += \
    legendpanel.ui \
    particlepanel.ui \
    renderoptionpanel.ui \
    timecontrolpanel.ui \
    transformpanel.ui \
    coordinatepanel.ui \
    systemstatuspanel.ui \
    coloreditdialog.ui \
    coloreditor.ui \
    opacityeditor.ui \
    transferfunceditor_$${PBVR_MODE}.ui \
    animationcontrols.ui \
    functionlisteditor.ui \
    filterinfopanel.ui \
    viewercontrols.ui \
    slidercontrol.ui


