#ifndef GLYPHITEM_H
#define GLYPHITEM_H

#include <QStandardItem>
#include "ColorMap.h"

class GlyphItem
{
public:
    enum GlyphType
    {
        Arrow   = 0, // Default
        Diamond = 1,
        Sphere  = 2,
    };

    enum DataDefines
    {
        Constant            = 0, //
        VariableArray       = 1  //
    };

    enum DistributionMode
    {
        UniformDistribution = 0, // Sampling Points, Seed
        AllPoints           = 1, // No UI Info(?)
        EveryNthPoints      = 2  // Stride
    };

    enum GlyphItemRole
    {
        UserGlyphType           = Qt::UserRole + 1,
        ScaleFactor             = Qt::UserRole + 2,
        Direction               = Qt::UserRole + 3,
        SizeDataDefines         = Qt::UserRole + 4,
        SizeNumberOfVariables   = Qt::UserRole + 5,
        SizeVariables           = Qt::UserRole + 6,
        DistributionMode        = Qt::UserRole + 7,
        NumberOfSamplePoints    = Qt::UserRole + 8,
        Seed                    = Qt::UserRole + 9,
        Stride                  = Qt::UserRole + 10,
        ColorMap                = Qt::UserRole + 11,
        ColorDataDefines        = Qt::UserRole + 12,
        ColorNumberOfVariables  = Qt::UserRole + 13,
        ColorVariables          = Qt::UserRole + 14,
    };
};

#endif // GLYPHITEM_H
