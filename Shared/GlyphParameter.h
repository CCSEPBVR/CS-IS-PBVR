#ifndef GLYPHPARAMETER_H
#define GLYPHPARAMETER_H

#include <iostream>
#include <vector>

#include <kvs/RGBColor>

class GlyphParameter
{
public:
    enum Type
    {
        Arrow   = 0,
        Diamond = 1,
        Sphere  = 2,
    };

    enum DataMode
    {
        Constant        = 0,
        VariableArray   = 1
    };

    enum DistributionMode
    {
        UniformDistribution = 0, // Sampling Points, Seed
        AllPoints           = 1, // No UI Info(?)
        EveryNthPoints      = 2  // Stride
    };
};

#endif // GLYPHPARAMETER_H
