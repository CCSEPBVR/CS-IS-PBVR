#=============================================================================
#  Template.
#=============================================================================
TEMPLATE = subdirs

#=============================================================================
#  Sub directorys.
#=============================================================================
SUBDIRS += Utils
SUBDIRS += FunctionParser
SUBDIRS += ExtendedKVS
SUBDIRS += ExtendedQT
SUBDIRS += Widgets
SUBDIRS += App
SUBDIRS += Tests

#=============================================================================
#  Depends.
#=============================================================================
App.depends = Widgets FunctionParser Utils ExtendedKVS ExtendedQT
Tests.depends = App Widgets FunctionParser Utils ExtendedKVS ExtendedQT
