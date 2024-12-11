#ifndef GLYPHPOLYGON_H
#define GLYPHPOLYGON_H
#include <kvs/PolygonObject>
#include "DataInitializer.h"
class GlyphPolygon
{
public:
    GlyphPolygon();
    kvs::PolygonObject* createArrowGlyphPolygon( const DataInitializer& data );
    kvs::PolygonObject* createDiamondGlyphPolygon( const DataInitializer& data );
    kvs::PolygonObject* createSphereGlyphPolygon( const DataInitializer& data );
};

#endif // GLYPHPOLYGON_H
