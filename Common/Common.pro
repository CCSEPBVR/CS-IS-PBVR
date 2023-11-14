include(../SETTINGS.pri)
#=============================================================================
#  Configuration valiable.
#=============================================================================
#CONFIG += release warn_off opengl
CONFIG += static
QT += core

greaterThan( QT_MAJOR_VERSION, 5 ) {
QT += openglwidgets
}
DEFINES += JPV_CLIENT



#=============================================================================
#  Template.
#=============================================================================
TEMPLATE = lib



#=============================================================================
#  Target.
#=============================================================================
TARGET = Common



#=============================================================================
#  Header.
#=============================================================================
HEADERS += \
ExtendedTransferFunctionParameter.h\
ParticleTransferClient.h\
ParticleTransferProtocol.h\
Types.h\
VariableRange.h\
serializer.h



#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
ExtendedTransferFunctionParameter.cpp\
ParticleTransferClient.cpp\
ParticleTransferProtocol.cpp\
VariableRange.cpp\
serializer.cpp



#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
