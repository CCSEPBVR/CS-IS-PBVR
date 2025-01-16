#ifndef __JUPITER_PLOT_OVER_LINE_FILE_H__
#define __JUPITER_PLOT_OVER_LINE_FILE_H__

#include <string>
#include <sstream>
#include <algorithm>
#include <kvs/Directory>
#include <kvs/File>
#include <kvs/Type>
#include "PointObject.h"
#include "PointImporter.h"
#include "KVSMLObjectPlotOverLine.h"
#include <kvs/GlyphObject>

class PlotOverLineFile
{
private:
    kvs::UInt32 m_subvolume_number ;
    kvs::UInt32 m_initial_step;
    kvs::UInt32 m_final_step;
    kvs::UInt32 m_kvsml_file_number;
    std::string m_file_prefix;

public:
    void setFilePrefix( const std::string& prefix );
    void setParameterFromFile();
    void generatePointObject( const int time_step, pbvr::PointObject* object );
    void generatePOLObject( const int time_step, kvs::KVSMLObjectPlotOverLine* object );
    int subVolumeNumber () {return m_subvolume_number;}
    int getStartTimeStep () {return m_initial_step;}
    int getLatestTimeStep () {return m_final_step;}
};

#endif
