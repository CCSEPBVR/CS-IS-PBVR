/****************************************************************************/
/**
 *  @file Module.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Module.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__MODULE_H_INCLUDE
#define VIS_MODULE__MODULE_H_INCLUDE

#define VIS_MODULE_MODULE_IMPORTER vismodule::ImporterBase::ModuleTag
#define VIS_MODULE_MODULE_FILTER   vismodule::FilterBase::ModuleTag
#define VIS_MODULE_MODULE_MAPPER   vismodule::MapperBase::ModuleTag
#define VIS_MODULE_MODULE_OBJECT   vismodule::ObjectBase::ModuleTag
#define VIS_MODULE_MODULE_RENDERER vismodule::RendererBase::ModuleTag

#define visModuleBase                           \
    public:                                     \
    struct  ModuleTag{};                        \
    typedef ModuleTag ModuleCategory

#define visModuleBaseClass( base_class )        \
    typedef base_class BaseClass

#define visModuleSuperClass( super_class )      \
    typedef super_class SuperClass

#define visModuleCategory( module_category )                            \
    public:                                                             \
    typedef vismodule:: module_category##Base::ModuleTag ModuleCategory

namespace vismodule
{

template <typename T>
struct ModuleTraits
{
    typedef typename T::ModuleCategory ModuleCategory;
};

} // end of namespace vismodule

#endif // VIS_MODULE__MODULE_H_INCLUDE
