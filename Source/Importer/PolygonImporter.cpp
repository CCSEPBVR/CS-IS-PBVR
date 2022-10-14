/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "PolygonImporter.h"

#include "FileFormat/STL/Stl.h"
#include "kvs/RGBColor"

namespace cvt
{

kvs::ObjectBase* PolygonImporter::exec( const kvs::FileFormatBase* file_format )
{
    BaseClass::setSuccess( false );

    if ( cvt::Stl::CheckExtension( file_format->filename() ) )
    {
        try
        {
            cvt::Stl ff( file_format->filename() );
            ff.read( file_format->filename() );
            import( ff );
            BaseClass::setSuccess( true );

            return this;
        }
        catch ( ... )
        {
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}

void PolygonImporter::import( const cvt::Stl& stl )
{
    stl.get( this );
    SuperClass::setColorType( kvs::PolygonObject::PolygonColor );
    SuperClass::setColor( kvs::RGBColor( 255, 255, 255 ) );
    SuperClass::setOpacity( 255 );
}
} // namespace cvt