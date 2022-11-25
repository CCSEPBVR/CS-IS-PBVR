/*
 * PbvrPointObjectGenerator.h
 *
 *  Created on: 2016/12/09
 *      Author: hira
 */

#ifndef PBVRPOINTOBJECTGENERATOR_H_
#define PBVRPOINTOBJECTGENERATOR_H_

#include "PointObjectGenerator.h"

namespace pbvr {

class PbvrPointObjectGenerator: public PointObjectGenerator {
public:
    PbvrPointObjectGenerator();
    virtual ~PbvrPointObjectGenerator();
    virtual void createFromFile(
        const Argument& param, const kvs::Camera& camera, const size_t subpixel_level, const float sampling_step, const int st, const int vl );

};

} /* namespace pbvr */

#endif /* PBVRPOINTOBJECTGENERATOR_H_ */
