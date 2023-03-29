TEMPLATE = lib
TARGET   = SupportQGlue
include( ../SETTINGS.pri)

## CONDITIONAL COMPILE ##
contains(DEFINES, SUPPORT_QGLUE){

message( BUILDING SupportQGlue FOR PLATFORM $${PLATFORM} $${RELTYPE} )

QT       += opengl
#QT       -= gui
CONFIG  += staticlib

SOURCES += \
    Renderer/EnsembleAverageBuffer.cpp \
    Renderer/ParticleVolumeRenderer.cpp \
    Renderer/PointRenderer.cpp \
    Renderer/PolygonRenderer.cpp \
#    Renderer/RayCastingRenderer.cpp \
#    Viewer/BufferObject.cpp \
    Viewer/FragmentShader.cpp \
    Viewer/FrameBufferObject.cpp \
    Viewer/GeometryShader.cpp \
    Viewer/IndexBufferObject.cpp \
    Viewer/PixelBufferObject.cpp \
    Viewer/ProgramObject.cpp \
    Viewer/RenderBuffer.cpp \
    Viewer/ShaderObject.cpp \
    Viewer/ShaderSource.cpp \
    Viewer/Texture3D.cpp \
    Viewer/TextureRectangle.cpp \
    Viewer/VertexBufferObject.cpp \
    Viewer/VertexShader.cpp

HEADERS += \
    Renderer/EnsembleAverageBuffer.h \
    Renderer/ParticleVolumeRenderer/Shader.h \
    Renderer/ParticleVolumeRenderer.h \
    Renderer/PointRenderer/Shader.h \
    Renderer/PointRenderer.h \
    Renderer/PolygonRenderer/Shader.h \
    Renderer/PolygonRenderer.h \
#    Renderer/RayCastingRenderer/Shader.h \
#    Renderer/RayCastingRenderer.h \
    Renderer/Shader/shading.h \
    Renderer/Shader/transfer_function.h \
    Renderer/Shader/volume.h \
#    Viewer/BufferObject.h \
    Viewer/FragmentShader.h \
    Viewer/FrameBufferObject.h \
    Viewer/GeometryShader.h \
    Viewer/IndexBufferObject.h \
    Viewer/PixelBufferObject.h \
    Viewer/ProgramObject.h \
    Viewer/RenderBuffer.h \
    Viewer/ShaderObject.h \
    Viewer/ShaderSource.h \
    Viewer/Texture3D.h \
    Viewer/TextureRectangle.h \
    Viewer/VertexBufferObject.h \
    Viewer/VertexShader.h

}
