inline float CalculateSamplingStep( const FilterInfoLst& fil )
{
    float max_coord_length = kvs::Math::Max<float>( fil.total_maxObjectCoord.x() - fil.total_minObjectCoord.x(),
                                                    fil.total_maxObjectCoord.y() - fil.total_minObjectCoord.y(),
                                                    fil.total_maxObjectCoord.z() - fil.total_minObjectCoord.z() );
    return 0.1 * max_coord_length;
}


