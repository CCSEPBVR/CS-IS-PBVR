#=============================================================================
#  sources and headers
#=============================================================================
EVENT_HEADERS += \
Event/EventTimer.h \


EVENT_SOURCES += \
Event/EventTimer.cpp \


HEADERS += $$EVENT_HEADERS
SOURCES += $$EVENT_SOURCES


#=============================================================================
#  install
#=============================================================================
EVENT_INSTALL_HEADERS.files = $$EVENT_HEADERS
EVENT_INSTALL_HEADERS.path  = $$KVS_DIR/include/SupportQt/Event

INSTALLS += Event_INSTALL_HEADERS
