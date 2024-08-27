#ifndef  __FILTER_INFO_H__
#define __FILTER_INFO_H__

#include <string>
#include <vector>
#include "Types.h"
#include <kvs/Vector3>

class FilterInfo
{
public:
    struct ingredientsMinMax
    {
        float min;
        float max;
    };
    struct ingredientsStep
    {
        std::vector<ingredientsMinMax> ingred;
    };

public:
    int32_t numNodes;
    int32_t numElements;
    int32_t elemType;
    int32_t fileType;
    int32_t numFiles;
    int32_t numIngredients;
    int32_t staSteps;
    int32_t endSteps;
    int32_t numSteps;
    int32_t numSubVolumes;
    kvs::Vector3f minObjectCoord;
    kvs::Vector3f maxObjectCoord;
    std::vector<kvs::Vector3f> minSubVolumeCoord;
    std::vector<kvs::Vector3f> maxSubVolumeCoord;
    float   minValue;
    float   maxValue;
    std::string filePath;
    std::vector<ingredientsStep> ingredStep;

public:
    int LoadPFI( std::string& filename );
};


class FilterInfoLst
{
public:
    FilterInfoLst();
    ~FilterInfoLst();

    int LoadPFL( std::string& filename );
    int GetFileIdx( int vl, int& xvl ) const;
    void CropTimestep( int s, int e );

    std::vector<FilterInfo> m_list;

    int32_t total_numNodes;
    int32_t total_numElements;
    int32_t total_numFiles;
    int32_t total_staSteps;
    int32_t total_endSteps;
    int32_t total_numSteps;
    int32_t total_numSubVolumes;
    kvs::Vector3f total_minObjectCoord;
    kvs::Vector3f total_maxObjectCoord;
    std::vector<kvs::Vector3f> total_minSubVolumeCoord;
    std::vector<kvs::Vector3f> total_maxSubVolumeCoord;
    float   total_minValue;
    float   total_maxValue;

    std::vector<FilterInfo::ingredientsStep> total_ingredStep;
};

#endif // __FILTER_INFO_H__

