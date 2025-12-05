/****************************************************************************/
/**
 *  @file PlotOverLineGenerator.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PlotOverLineGenerator.h 634 2025-01-31 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/

#ifndef VIS_MODULE__POL_GENERATOR_H_INCLUDE
#define VIS_MODULE__POL_GENERATOR_H_INCLUDE

#include <vector>
#include <string>
#include "UnstructuredVolumeObject.h"
#include <vismodule/MultiVolumeProperty>
#include <vismodule/ExtendedTransferFunction>
#include <vismodule/KVSMLObjectGlyph>
#include <vismodule/PlotOverLine>
#include <vismodule/KVSMLObjectPlotOverLine>

#include <sys/stat.h>
#include <vismodule/Camera>
#include <vismodule/Argument>


class PlotOverLineGenerator
{
private:

    vismodule::UnstructuredVolumeObject* m_volume;
    vismodule::KVSMLObjectPlotOverLine* m_object;


    int m_mpi_rank;


public:

    PlotOverLineGenerator()
        : m_volume(NULL), m_mpi_rank(0), m_mvp(NULL), m_object(NULL) {}

    ~PlotOverLineGenerator()
    {
    }

public:
    void GeneratePOLStruct(
        const Argument& param,
        const domain_parameters_struct& dom,
        Type** values,
        int nvariables,
        vismodule::KVSMLObjectPlotOverLine* object
    );

    void GeneratePOLUnstruct(
        const Argument& param,
        Type** values,
        int nvariables,
        float* coordinates,
        int ncoords,
        unsigned int* connections,
        int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype,
        vismodule::KVSMLObjectPlotOverLine* object
    );

    void run(
        const Argument& param,
        const int number_of_divide,
        vismodule::KVSMLObjectPlotOverLine* object
    )
    {
        delete  m_object;
        struct stat s;

        if ( stat( param.m_input_data.c_str(), &s ) )
        {
            std::cout << "Error. read failed:" << param.m_input_data << std::endl;
            exit( 1 );
        }

        m_object = new vismodule::KVSMLObjectPlotOverLine();
        this->createFromFile( param, number_of_divide );

        vismodule::KVSMLObjectPlotOverLine* po = getKVSMLObjectPOL();

        object->setValuesOnLine(po->values_on_line()); 
        object->setXAxis(po->x_axis()); 
        object->setMask(po->mask()); 
    }

    void run( 
        const Argument& param,
        const int number_of_divide,
        vismodule::KVSMLObjectPlotOverLine* object,
        const int st,
        const int vl
    )
    {
        delete  m_object;

        m_object = new vismodule::KVSMLObjectPlotOverLine();
        this->createFromFile( param, number_of_divide, st, vl );

        vismodule::KVSMLObjectPlotOverLine* po = getKVSMLObjectPOL();

        object->setValuesOnLine(po->values_on_line());
        object->setXAxis(po->x_axis());
        object->setMask(po->mask());
    }

//    vismodule::KVSMLObjectGlyph* run( const Argument& param, const vismodule::Camera& camera,const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide, const int timeStep, const int st, const int vl)
//    {
//        m_generator.setFinlterInfo( m_mvp );
//        m_generator.setCoordSynthTS( st );
//        m_generator.createFromFile( param, camera, clntMes, number_of_divide, st, vl );
//        vismodule::KVSMLObjectGlyph* po = m_generator.getKVSMLObjectGlyph();
//        return po;
//    }

protected:

private:
    //vismodule::KVSMLObjectGlyph* m_object;
    
    vismodule::ValueArray<vismodule::Real32> m_coords;       ///< coordinate array
    vismodule::ValueArray<vismodule::UInt8>  m_colors;       ///< color(r,g,b) array
    vismodule::ValueArray<vismodule::Real32> m_directions;   ///< directions array
    vismodule::ValueArray<vismodule::Real32> m_sizes;        ///< size array


    const MultiVolumeProperty*   m_mvp;

public:


    void createFromFile(
        const Argument& param,
        const int number_of_divide
    );
        //const Argument& param, const vismodule::Camera& camera);

    void createFromFile(
        const Argument& param,
        const int number_of_divide,
        const int st,
        const int vl
    );

//    void createFromFile(
//        const Argument& param, const vismodule::Camera& camera, const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide, const int st, const int vl );

    vismodule::KVSMLObjectPlotOverLine* getKVSMLObjectPOL()
    {
        //return &m_object;
        return m_object;
    }

    std::string getErrorMessage( const size_t maxMemory ) const;

    void setFinlterInfo( const MultiVolumeProperty *mvp )
    {
        m_mvp = mvp;
    }

public:

    const vismodule::ValueArray<vismodule::Real32>& coords( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& colors( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& directions( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& sizes( void ) const;

public:

    void setCoords( const vismodule::ValueArray<vismodule::Real32>& coords );

    void setColors( const vismodule::ValueArray<vismodule::UInt8>& colors );

    void setDirections( const vismodule::ValueArray<vismodule::Real32>& deirections );
    
    void setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes );

    void clear();
private:
//    void sampling( vismodule::VolumeObjectBase* volume, const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide);
    
    template <typename T>
        void copy_values(vismodule::AnyValueArray& valueArray, std::unique_ptr<std::unique_ptr<Type[]>[]>& values, int nvariables, int nnodes);


};


#endif
