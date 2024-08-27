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
#ifndef KVS__CLASS_NAME_H_INCLUDE
#define KVS__CLASS_NAME_H_INCLUDE

#define kvsClassName( this_class )                \
    public:                                       \
    virtual const char* className() const { \
        return # this_class;                  \
    }

#define kvsClassName_without_virtual( this_class ) \
    public:                                        \
    const char* className() const {          \
        return # this_class;                   \
    }

#endif // PBVR__KVS__CLASS_NAME_H_INCLUDE
