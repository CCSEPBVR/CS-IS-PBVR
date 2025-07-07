#ifndef UTILS_H
#define UTILS_H

#include <QString>

#include <kvs/Vector>
struct ServerPointObjectPropertiesCS
{
    int numberOfIngredients;
    int numberOfElements;
    int numberOfVolumeDivide;
    int numberOfNodes;
    int elementType;
    int fileType;
    int numberOfStep;
    kvs::Vec3f minObjectCoords;
    kvs::Vec3f maxObjectCoords;
    int minTimeStep;
    int maxTimeStep;
};

struct ServerGlyphObjectPropertiesCS
{
    kvs::Vec3f minObjectCoords;
    kvs::Vec3f maxObjectCoords;
    int minTimeStep;
    int maxTimeStep;
};

struct ServerPointObjectPropertiesIS
{
    int numberOfIngredients;
    kvs::Vec3f minObjectCoords;
    kvs::Vec3f maxObjectCoords;
    int minTimeStep;
    int maxTimeStep;
    float particleLimit;
    float particleDensity;
};

struct ServerGlyphObjectPropertiesIS
{
    kvs::Vec3f minObjectCoords;
    kvs::Vec3f maxObjectCoords;
    int minTimeStep;
    int maxTimeStep;
};

/**
 * @class Utils
 * @brief 補助関数を提供するクラス
 */
class Utils
{
public:
    static std::string toNativePath( const QString& path );
};

#endif // UTILS_H
