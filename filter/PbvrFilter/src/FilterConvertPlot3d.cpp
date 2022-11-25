/*
 * FilterConvertVtk.cpp
 *
 *  Created on: 2016/11/21
 *      Author: hira
 */
#include <float.h>

#include "FilterConvertPlot3d.h"
#include "filter_log.h"
#include "filter_utils.h"
#include "filter_tree.h"
#include "filter_io_plot3d/FilterIoPlot3d.h"

namespace pbvr
{
namespace filter
{


FilterConvertPlot3d::FilterConvertPlot3d(PbvrFilterInfo *filter_info, PbvrFilter *parent)
    : FilterConvert(filter_info, parent) {

}


FilterConvertPlot3d::~FilterConvertPlot3d() {

}


int FilterConvertPlot3d::initializePlot3d(FilterIoPlot3d *io_plot3d) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    io_plot3d->allocationList_create(2000);
    printf("Initializing\n");
    fflush(stdout);
    ASSERT_RTC_OK(io_plot3d->read_plot3d_config(), "Could not read plot3d config.");
    io_plot3d->plot3d_initOffsetValueArray();
    printf("Reading Grid Coordinates\n");
    fflush(stdout);
    io_plot3d->plot3d_readGridCoordinateFile();
    fflush(stdout);
    info->number_of_blocks = io_plot3d->data_set->nBlocks;

    return RTC_OK;
}

int FilterConvertPlot3d::finalizePlot3d(FilterIoPlot3d *io_plot3d) {

    io_plot3d->plot3d_dataSet_deallocate(io_plot3d->data_set);
    io_plot3d->allocationList_destruct();
    return RTC_OK;
}


/**
 * This is the entry point from the main() process.
 * @param argc
 * @param argv
 * @return
 */
//int FilterConvertPlot3d::plot3d_sequentialConvert() {
int FilterConvertPlot3d::filter_convert() {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    this->initializePlot3d(this->m_io_plot3d);

    for (info->current_block = 0; info->current_block < this->m_io_plot3d->data_set->nBlocks; info->current_block++) {
        printf("\r%80s\r", " ");
        printf("Converting block %i of %i => ", info->current_block+1, this->m_io_plot3d->data_set->nBlocks);
        FilterConvert::filter_convert();
    }
    this->finalizePlot3d(this->m_io_plot3d);

    return RTC_OK;
}



} // end of namespace filter
} // end of namespace pbvr
