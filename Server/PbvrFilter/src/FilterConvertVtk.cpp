/*
 * FilterConvertVtk.cpp
 *
 *  Created on: 2016/11/21
 *      Author: hira
 */
#include <float.h>

#include "FilterConvertVtk.h"
#include "filter_utils.h"
#include "filter_tree.h"
#include "FilterWrite.h"
#include "filter_io_avs/FilterIoAvs.h"
#include "filter_io_plot3d/FilterIoPlot3d.h"
#include "filter_io_stl/FilterIoStl.h"


namespace pbvr
{
namespace filter
{


FilterConvertVtk::FilterConvertVtk(PbvrFilterInfo *filter_info,  PbvrFilter *parent)
    : FilterConvert(filter_info, parent) {

}


FilterConvertVtk::~FilterConvertVtk() {

}


RTC FilterConvertVtk::get_datasetInfo() {
#ifdef VTK_ENABLED
    this->m_io_vtk->vtk_get_datasetInfo();
#endif
    return RTC_OK;
}

int FilterConvertVtk::filter_convert() {
    int status = this->get_datasetInfo();
    status |= FilterConvert::filter_convert();

    return status;
}

} // end of namespace filter
} // end of namespace pbvr
