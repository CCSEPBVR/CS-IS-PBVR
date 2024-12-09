/****************************************************************************/
/**
 *  @file ClassName.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ClassName.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__CLASS_NAME_H_INCLUDE
#define VIS_MODULE__CLASS_NAME_H_INCLUDE

#define visModuleClassName( this_class )                \
    public:                                       \
    virtual const char* className() const { \
        return # this_class;                  \
    }

#define visModuleClassName_without_virtual( this_class ) \
    public:                                        \
    const char* className() const {          \
        return # this_class;                   \
    }

#endif // VIS_MODULE__CLASS_NAME_H_INCLUDE
