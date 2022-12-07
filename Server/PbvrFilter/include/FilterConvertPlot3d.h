/*
 * FilterConvertPlot3d.h
 *
 *  Created on: 2016/11/21
 *      Author: hira
 */

#ifndef FILTERCONVERTPLOT3D_H_
#define FILTERCONVERTPLOT3D_H_

#include "FilterConvert.h"

namespace pbvr
{
namespace filter
{

class PbvrFilter;
class FilterConvertPlot3d: public FilterConvert {
public:
    FilterConvertPlot3d(PbvrFilterInfo *filter_info, PbvrFilter *parent);
    virtual ~FilterConvertPlot3d();
    virtual int filter_convert();
    int initializePlot3d(FilterIoPlot3d *io_plot3d);
    int finalizePlot3d(FilterIoPlot3d *io_plot3d);
private:

};

} // end of namespace filter
} // end of namespace pbvr
#endif /* FILTERCONVERTPLOT3D_H_ */
