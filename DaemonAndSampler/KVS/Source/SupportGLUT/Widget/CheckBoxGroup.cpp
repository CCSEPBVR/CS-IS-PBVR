/*****************************************************************************/
/**
 *  @file   CheckBoxGroup.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CheckBoxGroup.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "CheckBoxGroup.h"
#include "CheckBox.h"


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new CheckBoxGroup class.
 *  @param  screen [in] pointer to the parent screen
 */
/*===========================================================================*/
CheckBoxGroup::CheckBoxGroup( kvs::ScreenBase* screen ):
    kvs::glut::WidgetBase( screen )
{
    BaseClass::setEventType(
        kvs::EventBase::PaintEvent |
        kvs::EventBase::ResizeEvent |
        kvs::EventBase::MousePressEvent |
        kvs::EventBase::MouseReleaseEvent );
}

/*===========================================================================*/
/**
 *  @brief  Returns the check box list.
 */
/*===========================================================================*/
const std::list<kvs::glut::CheckBox*>& CheckBoxGroup::checkBoxes( void ) const
{
    return( m_boxes );
}

/*===========================================================================*/
/**
 *  @brief  Adds a check box.
 *  @param  box [in] pointer to the check box
 */
/*===========================================================================*/
void CheckBoxGroup::add( kvs::glut::CheckBox* box )
{
    box->attach_group( this );
    m_boxes.push_back( box );
}

/*===========================================================================*/
/**
 *  @brief  Removes the check box.
 *  @param  box [in] pointer to the check box
 */
/*===========================================================================*/
void CheckBoxGroup::remove( kvs::glut::CheckBox* box )
{
    box->detach_group();
    m_boxes.remove( box );
}

/*===========================================================================*/
/**
 *  @brief  Shows the boxes in the group.
 */
/*===========================================================================*/
void CheckBoxGroup::show( void )
{
    BaseClass::show();

    std::list<kvs::glut::CheckBox*>::iterator box = m_boxes.begin();
    std::list<kvs::glut::CheckBox*>::iterator last = m_boxes.end();
    while ( box != last )
    {
        (*box++)->show();
    }
}

/*===========================================================================*/
/**
 *  @brief  Hides the boxes in the group.
 */
/*===========================================================================*/
void CheckBoxGroup::hide( void )
{
    BaseClass::hide();

    std::list<kvs::glut::CheckBox*>::iterator box = m_boxes.begin();
    std::list<kvs::glut::CheckBox*>::iterator last = m_boxes.end();
    while ( box != last )
    {
        (*box++)->hide();
    }
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void CheckBoxGroup::paintEvent( void )
{
    this->screenUpdated();
}

/*===========================================================================*/
/**
 *  @brief  Resize event.
 *  @param  width [in] resized screen width
 *  @param  height [in] resized screen height
 */
/*===========================================================================*/
void CheckBoxGroup::resizeEvent( int width, int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );

    this->screenResized();
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void CheckBoxGroup::mousePressEvent( kvs::MouseEvent* event )
{
    if ( !BaseClass::isShown() ) return;

    int id = 0;
    std::list<kvs::glut::CheckBox*>::iterator box = m_boxes.begin();
    std::list<kvs::glut::CheckBox*>::iterator last = m_boxes.end();
    while ( box != last )
    {
        if ( (*box)->contains( event->x(), event->y() ) )
        {
            BaseClass::activate();
            this->pressed( *box );
            this->pressed( id );
        }

        box++;
        id++;
    }
}

/*===========================================================================*/
/**
 *  @brief  Mouse release event.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void CheckBoxGroup::mouseReleaseEvent( kvs::MouseEvent* event )
{
    if ( !BaseClass::isShown() ) return;

    if ( BaseClass::isActive() )
    {
        int id = 0;
        std::list<kvs::glut::CheckBox*>::iterator box = m_boxes.begin();
        std::list<kvs::glut::CheckBox*>::iterator last = m_boxes.end();
        while ( box != last )
        {
            if ( (*box)->contains( event->x(), event->y() ) )
            {
                this->released( *box );
                this->released( id );
                BaseClass::deactivate();
            }

            box++;
            id++;
        }
    }
}

} // end of namespace glut

} // end of namespace kvs
