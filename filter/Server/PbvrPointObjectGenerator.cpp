/*
 * PbvrPointObjectGenerator.cpp
 *
 *  Created on: 2016/12/09
 *      Author: hira
 */

#include "PbvrFilterInformation.h"
#include "PbvrPointObjectGenerator.h"
#include "timer_simple.h"
#include "UnstructuredVolumeImporter.h"

namespace pbvr {

PbvrPointObjectGenerator::PbvrPointObjectGenerator() : PointObjectGenerator() {

}

PbvrPointObjectGenerator::~PbvrPointObjectGenerator() {
    // TODO Auto-generated destructor stub
}


void PbvrPointObjectGenerator::createFromFile(
        const Argument& param, const kvs::Camera& camera, const size_t subpixel_level, const float sampling_step, const int st, const int vl )
{

    PBVR_TIMER_STA( 260 );
    if (m_object != NULL) delete m_object;

    pbvr::UnstructuredVolumeObject* volume = NULL;

    const PbvrFilterInformationFile *pbvr_fi = dynamic_cast<const PbvrFilterInformationFile *>(m_fi);
    if (pbvr_fi != NULL && pbvr_fi->getPbvrFilter() != NULL) {
        volume = new pbvr::UnstructuredVolumeImporter( pbvr_fi, st, vl );
    }
    if ( volume )
    {
        volume->setCoordSynthesizerStrings( m_coord_synthesizer_strings );
    }

    PBVR_TIMER_END( 260 );

    volume->setMinMaxValues( m_fi->m_min_value, m_fi->m_max_value );
    volume->setMinMaxObjectCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );
    volume->setMinMaxExternalCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord() << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    try
    {
        m_object = sampling( param, camera, volume, subpixel_level, sampling_step );
    }
    catch ( const std::runtime_error& e )
    {
        m_object = NULL;
        if (volume != NULL) delete volume;

        // add by @hira at 3016/12/01
        fprintf(stderr, "[%s::%d] runtime_error::%s \n",
                __FILE__, __LINE__, e.what());

        throw e;
    }

    if (volume != NULL) delete volume;

    return;
}

} /* namespace pbvr */
