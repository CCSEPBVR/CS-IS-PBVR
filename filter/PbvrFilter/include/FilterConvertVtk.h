/*
 * FilterConvertVtk.h
 *
 *  Created on: 2016/11/21
 *      Author: hira
 */

#ifndef FILTERCONVERTVTK_H_
#define FILTERCONVERTVTK_H_

#include "FilterConvert.h"
#ifdef VTK_ENABLED
#include "filter_io_vtk/FilterIoVtk.h"
#endif

namespace pbvr
{
namespace filter
{

class PbvrFilter;
class FilterConvertVtk: public FilterConvert {
public:
    FilterConvertVtk(PbvrFilterInfo *filter_info, PbvrFilter *parent);
    virtual ~FilterConvertVtk();
    virtual int filter_convert();
    RTC get_datasetInfo();

private:

};

} // end of namespace filter
} // end of namespace pbvr
#endif /* FILTERCONVERTVTK_H_ */
