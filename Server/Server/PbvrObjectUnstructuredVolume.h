/*
 * PbvrObjectUnstructuredVolume.h
 *
 *  Created on: 2016/12/07
 *      Author: hira
 */

#ifndef PBVROBJECTUNSTRUCTUREDVOLUME_H_
#define PBVROBJECTUNSTRUCTUREDVOLUME_H_

#include "PbvrFilterInformation.h"
#include "FileFormat/KVSML/KVSMLObjectUnstructuredVolume.h"

namespace pbvr {

class PbvrObjectUnstructuredVolume : public kvs::KVSMLObjectUnstructuredVolume {
public:
    kvsClassName( pbvr::PbvrObjectUnstructuredVolume );
    PbvrObjectUnstructuredVolume();
    PbvrObjectUnstructuredVolume( const PbvrFilterInformationFile *pbvr_fi, int step, int subvolume);
    virtual ~PbvrObjectUnstructuredVolume();

    const bool readValue(const PbvrFilterInformationFile *pbvr_fi, int step, int subvolume);
    const bool readCoords(const PbvrFilterInformationFile *pbvr_fi, int step, int subvolume);
    const bool readConnection(const PbvrFilterInformationFile *pbvr_fi, int step, int subvolume);

};

} /* namespace pbvr */

#endif /* PBVROBJECTUNSTRUCTUREDVOLUME_H_ */
