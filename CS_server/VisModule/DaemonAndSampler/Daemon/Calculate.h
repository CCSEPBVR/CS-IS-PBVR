//inline float CalculateSamplingStep( const FilterInfoLst& fil )
inline float CalculateSamplingStep( const MultiVolumePropertyList& mvpl )
{
    float max_coord_length = kvs::Math::Max<float>( mvpl.total_maxObjectCoord.x() - mvpl.total_minObjectCoord.x(),
                                                    mvpl.total_maxObjectCoord.y() - mvpl.total_minObjectCoord.y(),
                                                    mvpl.total_maxObjectCoord.z() - mvpl.total_minObjectCoord.z() );
    return 0.1 * max_coord_length;
}


