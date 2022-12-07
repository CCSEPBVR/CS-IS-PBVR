/*
 * PbvrJobDispatcher.h
 *
 *  Created on: 2017/01/08
 *      Author: hira
 */

#ifndef PBVRJOBDISPATCHER_H_
#define PBVRJOBDISPATCHER_H_

#include "JobDispatcher.h"
#include "FilterInformation.h"

class PbvrJobDispatcher: public JobDispatcher {
public:
    PbvrJobDispatcher();
    virtual ~PbvrJobDispatcher();
    void setFilterInformationList(const FilterInformationList *fil);
    virtual int dispatchNext( const int worker_id, int* step, int* volume );

private:
    const FilterInformationList *m_filterinfos;
};

#endif /* PBVRJOBDISPATCHER_H_ */
