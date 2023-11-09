TARGET   = QGlue
TEMPLATE = lib
CONFIG  += static
QT += core gui opengl widgets openglwidgets

#PRECOMPILED_HEADER=stable.h

include( ../SETTINGS.pri)
message (Building PBVR:$${TARGET} -  Mode:$${PBVR_MODE}  - Platform: $${PLATFORM}  - ReleaseType: $${RELTYPE})

CONFIG += warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

HEADERS += \
    colorpalette.h \
    colormappalette.h \
    renderarea.h \
    orientationaxis.h \
    qglue_base.h \
    timer.h \
    eventlistener.h \
    opacitymappalette.h \
    histogram.h \
    labelbase.h \
    legendbar.h \
    extCommand.h \
    typedSignalConnect.h \
    systeminfo.h \
    colorundoredocommand.h \
    opacityundoredocommand.h \
    deferredtexture.h \
    Scene.h

HEADERS += stb_image_write.h

SOURCES +=     \
    colorpalette.cpp \
    colormappalette.cpp \
    renderarea.cpp \
    orientationaxis.cpp \
    qglue_base.cpp \
    timer.cpp \
    eventlistener.cpp \
    opacitymappalette.cpp \
    histogram.cpp \
    labelbase.cpp \
    legendbar.cpp \
    extCommand.cpp \
    systeminfo.cpp \
    colorundoredocommand.cpp \
    opacityundoredocommand.cpp \
    deferredtexture.cpp \
    Scene.cpp

HEADERS += ObjectManager.h
SOURCES += ObjectManager.cpp
HEADERS += lineobjectproxy.h
HEADERS += polygonobjectproxy.h
HEADERS += texturedpolygonobjectproxy.h
HEADERS += TimeMeasureUtility.h
HEADERS += boundingbox.h
SOURCES += boundingbox.cpp
HEADERS += objnameutil.h
SOURCES += objnameutil.cpp
HEADERS += timemeasure.h
SOURCES += timemeasure.cpp
HEADERS += StochasticRenderingCompositor.h
SOURCES += StochasticRenderingCompositor.cpp
HEADERS += StochasticLineRenderer.h
SOURCES += StochasticLineRenderer.cpp
HEADERS += StochasticPolygonRenderer.h
SOURCES += StochasticPolygonRenderer.cpp
HEADERS += stochasticrenderingenginecustom.h
SOURCES += stochasticrenderingenginecustom.cpp
HEADERS += stochastictexturedpolygonrenderer.h
SOURCES += stochastictexturedpolygonrenderer.cpp
HEADERS += qcolormaplibrary.h
SOURCES += qcolormaplibrary.cpp

contains(DEFINES, ENABLE_LIBLAS) {
HEADERS += LASImporter.h
SOURCES += LASImporter.cpp
}

contains(DEFINES, DISP_MODE_2D) {
    HEADERS += screen.h
    SOURCES += screen.cpp
}

contains(DEFINES, DISP_MODE_VR) {
    HEADERS += screen_vr.h
    SOURCES += screen_vr.cpp
    
    HEADERS += oculus.h
    HEADERS += typedSignalConnect.h
    HEADERS += version.h
    HEADERS += objectscale.h

    HEADERS += call.h
    SOURCES += call.cpp
    HEADERS += controllerbase.h
    SOURCES += controllerbase.cpp
    HEADERS += headmounteddisplay.h
    SOURCES += headmounteddisplay.cpp
    HEADERS += inputdevice.h
    SOURCES += inputdevice.cpp
    HEADERS += oculustexturebuffer.h
    SOURCES += oculustexturebuffer.cpp
    HEADERS += ovr.h
    SOURCES += ovr.cpp
    HEADERS += pbvrwidgethandler.h
    SOURCES += pbvrwidgethandler.cpp
    HEADERS += touchcontroller.h
    SOURCES += touchcontroller.cpp
    HEADERS += widgetbase.h
    SOURCES += widgetbase.cpp
    HEADERS += widgethandler.h
    SOURCES += widgethandler.cpp

    contains(DEFINES, MR_MODE) {
        HEADERS += cameraimagerenderer.h
        SOURCES += cameraimagerenderer.cpp
        HEADERS += stereocamera.h
        SOURCES += stereocamera.cpp
    }
}

