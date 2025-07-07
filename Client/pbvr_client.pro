#=============================================================================
#  Template.
#=============================================================================
TEMPLATE = subdirs

#=============================================================================
#  Sub directorys.
#=============================================================================
SUBDIRS += Utils
SUBDIRS += Common
SUBDIRS += FunctionParser
SUBDIRS += ExtendedKVS
SUBDIRS += ExtendedQT
SUBDIRS += Widgets
SUBDIRS += App

#=============================================================================
#  Depends.
#=============================================================================
App.depends = Widgets FunctionParser Common Utils ExtendedKVS ExtendedQT
