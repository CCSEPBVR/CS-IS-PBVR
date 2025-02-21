/****************************************************************************/
/**
 *  @file POLObjectGenerator.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: POLObjectGenerator.h 634 2025-01-31 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/

#ifndef KVS__POL_OBJECT_GENERATOR_H_INCLUDE
#define KVS__POL_OBJECT_GENERATOR_H_INCLUDE

#include <vector>
#include <string>
#include "UnstructuredVolumeObject.h"
#include "FilterInformation.h"
#include "ExtendedTransferFunction.h"
#include "timer.h"
#include "GlyphObjectGenerator.h"
#include "KVSMLObjectGlyph.h"
#include "../Common/ParticleTransferProtocol.h"
#include "PlotOverLine.h"
#include "KVSMLObjectPlotOverLine.h"

#include <sys/stat.h>
#include <kvs/Camera>
#include "Argument.h"


class POLObjectGenerator
{
private:

    pbvr::UnstructuredVolumeObject* m_volume;
    kvs::KVSMLObjectPlotOverLine* m_object;

//    kvs::KVSMLPlotOverLine m_generator;

    int m_mpi_rank;


public:

    POLObjectGenerator()
        : m_volume(NULL), m_mpi_rank(0), m_fi(NULL), m_object(NULL) {}

    ~POLObjectGenerator()
    {
    }

public:

    void run( const Argument& param, const kvs::Camera& camera, const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide ,const int timeStep, kvs::KVSMLObjectPlotOverLine* object, const int st = 1 )
    {

        delete  m_object;
        struct stat s;
        if ( stat( param.m_input_data.c_str(), &s ) )
        {
            std::cout << "Error. read failed:" << param.m_input_data << std::endl;
            exit( 1 );
        }
            m_object = new kvs::KVSMLObjectPlotOverLine();
            this -> createFromFile( param, camera, clntMes, number_of_divide);

        kvs::KVSMLObjectPlotOverLine* po = getKVSMLObjectPOL();

        object->setValuesOnLine(po->values_on_line()); 
        object->setXAxis(po->x_axis()); 
        object->setMask(po->mask()); 
    }

//    kvs::KVSMLObjectGlyph* run( const Argument& param, const kvs::Camera& camera,const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide, const int timeStep, const int st, const int vl)
//    {
//        m_generator.setFinlterInfo( m_fi );
//        m_generator.setCoordSynthTS( st );
//        m_generator.createFromFile( param, camera, clntMes, number_of_divide, st, vl );
//        kvs::KVSMLObjectGlyph* po = m_generator.getKVSMLObjectGlyph();
//        return po;
//    }

protected:

private:
    //kvs::KVSMLObjectGlyph* m_object;
    
    kvs::ValueArray<kvs::Real32> m_coords;       ///< coordinate array
    kvs::ValueArray<kvs::UInt8>  m_colors;       ///< color(r,g,b) array
    kvs::ValueArray<kvs::Real32> m_directions;   ///< directions array
    kvs::ValueArray<kvs::Real32> m_sizes;        ///< size array


    const FilterInformationFile*   m_fi;

public:


    void createFromFile(
        const Argument& param, const kvs::Camera& camera, const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide);
        //const Argument& param, const kvs::Camera& camera);

//    void createFromFile(
//        const Argument& param, const kvs::Camera& camera, const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide, const int st, const int vl );

    kvs::KVSMLObjectPlotOverLine* getKVSMLObjectPOL()
    {
        //return &m_object;
        return m_object;
    }

    std::string getErrorMessage( const size_t maxMemory ) const;

    void setFinlterInfo( const FilterInformationFile *fi )
    {
        m_fi = fi;
    }

public:

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::UInt8>& colors( void ) const;

    const kvs::ValueArray<kvs::Real32>& directions( void ) const;

    const kvs::ValueArray<kvs::Real32>& sizes( void ) const;

public:

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setColors( const kvs::ValueArray<kvs::UInt8>& colors );

    void setDirections( const kvs::ValueArray<kvs::Real32>& deirections );
    
    void setSizes( const kvs::ValueArray<kvs::Real32>& sizes );

    void clear();
private:
//    void sampling( pbvr::VolumeObjectBase* volume, const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide);

};


#endif
