/*****************************************************************************/
/**
 *  @file   Label.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Label.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLUT__LABEL_H_INCLUDE
#define KVS__GLUT__LABEL_H_INCLUDE

#include <string>
#include <vector>
#include <kvs/ClassName>
#include <kvs/glut/Screen>
#include "WidgetBase.h"


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Label class.
 */
/*===========================================================================*/
class Label : public kvs::glut::WidgetBase
{
    // Class name.
    kvsClassName( kvs::glut::Label );

public:

    typedef kvs::glut::WidgetBase BaseClass;

protected:

    std::vector<std::string> m_text; ///< text list

public:

    Label( kvs::ScreenBase* screen = 0 );

public:

    virtual void screenUpdated( void ){};

    virtual void screenResized( void ){};

public:

    void setText( const char* text, ... );

    void addText( const char* text, ... );

private:

    int get_fitted_width( void );

    int get_fitted_height( void );

public:

    void paintEvent( void );

    void resizeEvent( int width, int height );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__LABEL_H_INCLUDE
