#include "PlotOverLine.h"
#include <vismodule/KVSMLObjectPlotOverLine>
#include <fstream>

PlotOverLine::PlotOverLine( void ){}

PlotOverLine::PlotOverLine( const vismodule::StructuredVolumeObject* volume,
                            const size_t resolution,
                            const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    this->setVolume( volume );

    this->setResolution( resolution );

    this->extractPlotLineStructured( P0, P1 );
}

PlotOverLine::PlotOverLine( const vismodule::UnstructuredVolumeObject* volume,
                            const size_t resolution,
                            const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    
    this->setVolume( volume );

    this->setResolution( resolution );

    this->extractPlotLine( P0, P1 );
}
// CS用コンストラクター
PlotOverLine::PlotOverLine( const vismodule::UnstructuredVolumeObject* volume,
                            const size_t resolution,
                            const vismodule::Vec3 P0, const vismodule::Vec3 P1, const int plot_variable ):
    m_plot_variable(plot_variable)
{
    
    this->setVolume( volume );

    this->setResolution( resolution );

    this->extractPlotLine( P0, P1 );
}

// CS用コンストラクター
PlotOverLine::PlotOverLine( const vismodule::StructuredVolumeObject* volume,
                            const size_t resolution,
                            const vismodule::Vec3 P0, const vismodule::Vec3 P1, const int plot_variable ):
    m_plot_variable(plot_variable)
{
    
    this->setVolume( volume );

    this->setResolution( resolution );

    this->extractPlotLineStructured( P0, P1 );
}

PlotOverLine::PlotOverLine( const POL::Polyhedron* volume,
                            const size_t resolution,
                            const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    this->setVolume( volume );

    this->setResolution( resolution );

    this->extractPlotLine( P0, P1 );
}

PlotOverLine::~PlotOverLine()
{
//    m_values_on_line.deallocate();
//    m_x_axis.deallocate();
//    m_mask.deallocate();
}

//void PlotOverLine::setVolume( const vismodule::StructuredVolumeObject* volume )
void PlotOverLine::setVolume( const vismodule::StructuredVolumeObject* volume )
{
    m_structured_volume = volume;
}


void PlotOverLine::setVolume( const vismodule::UnstructuredVolumeObject* volume )
{
    m_volume = volume;
}

void PlotOverLine::setVolume( const POL::Polyhedron* volume )
{
    m_polyhedron = volume;
}

void PlotOverLine::setResolution( const size_t resolution )
{
    m_values_on_line.allocate( resolution );
    m_values_on_line.fill( 0x00 );

    m_x_axis.allocate( resolution );
    m_x_axis.fill( 0x00 );

    m_mask.allocate( resolution );
    m_mask.fill( false );

    m_allcell_values_on_line.allocate(resolution);
    m_allcell_values_on_line.fill( 0x00 );

    m_allcell_mask.allocate(resolution);
    m_allcell_mask.fill( false );

}

void PlotOverLine::extractPlotLineStructured( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    this->calculate_x_axis( P0, P1 );

    this->for_structured_mesh( P0, P1 );
}

bool PlotOverLine::SetPOLParameter( const int time_step )
{
    std::string visParamDir;
    std::string POLParamPath;
    std::string POLFilePath;

    const char *envBuf = NULL;
    envBuf = std::getenv( "VIS_PARAM_DIR" );
    if (envBuf == NULL) {
        visParamDir = "./";
    }
    else {
        visParamDir = envBuf;
        if (visParamDir[visParamDir.size() - 1] != '/') {
            visParamDir += "/";
        }
    }
    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        POLFilePath = "./p_";
    }
    else {
        POLFilePath = envBuf;
        if (POLFilePath[POLFilePath.size() - 1] != '/') {
            POLFilePath += "/p_";
        }
        else {
            POLFilePath += "p_";
        }
    }

    POLParamPath = visParamDir + "parameter.pol";

    m_POLParamPath = POLParamPath;
    m_POLFilePath = POLFilePath;

    PlotOverLineProperty plot_over_line_property;

    bool read_flag =  plot_over_line_property.LoadIN(POLParamPath) ;

    int mpi_rank;
#ifndef CPU_VER 
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif
    if(read_flag)
    {

        bool plot_flag;
        std::string              p_flag                    = plot_over_line_property.getString( "PLOT_FLAG" );
        int resolution                                     = plot_over_line_property.getInt("SAMPLING_SIZE");
        std::string              p_variable                = plot_over_line_property.getString( "PLOT_VARIABLE" );
        m_plot_variable = std::atoi(p_variable.substr(1).c_str()) -1;
        std::vector<float> s_table;
        s_table = plot_over_line_property.getTableFloat( "START_POINT" );
        std::vector<float> e_table;
        e_table = plot_over_line_property.getTableFloat( "END_POINT" );

        if(strcmp(p_flag.c_str(), "TRUE") ==0 ) plot_flag = true;
        else plot_flag = false;
        m_resolution = resolution;
        m_start_point.x() = s_table[0];
        m_start_point.y() = s_table[1];
        m_start_point.z() = s_table[2];
        m_end_point.x() = e_table[0];
        m_end_point.y() = e_table[1];
        m_end_point.z() = e_table[2];
        if (plot_flag || m_resolution > 0)this->setResolution( m_resolution );
        m_plot_flag = plot_flag;
    }
    else
    {
        m_plot_flag = false;
    }
    return m_plot_flag; 

}

void PlotOverLine::extractPlotLine( const vismodule::UnstructuredVolumeObject* volume)
{
    this->setVolume( volume );
    this->extractPlotLine( m_start_point, m_end_point );
}

void PlotOverLine::CellTypeReduceing()
{
    //reduce data
    for (int i =0; i<m_resolution; i++)
    {
        //polData.m_x_axis       [i]  = plot_over_line->xAxis() [i];
        if (m_mask[i] )
        {
            m_allcell_values_on_line[i] = m_values_on_line[i];
            m_allcell_mask          [i] = m_mask  [i];
        } 
    }

}

void PlotOverLine::extractPlotLine( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    this->calculate_x_axis( P0, P1 );

    switch( m_volume->cellType() )
    {
        case vismodule::UnstructuredVolumeObject::Tetrahedra:
        {
            this->for_tetrahedral_mesh( P0, P1 );
            break;
        }
        case vismodule::UnstructuredVolumeObject::Hexahedra:
        {
            this->for_hexahedral_mesh( P0, P1 );
            break;
        }
        case vismodule::UnstructuredVolumeObject::Pyramid:
        {
            this->for_pyramidal_mesh( P0, P1 );
            break;
        }
        case vismodule::UnstructuredVolumeObject::Prism:
        {
            this->for_prismic_mesh( P0, P1 );
            break;
        }
        default:
        {
            break;
        }
    }
}

void PlotOverLine::extractPlotLinePoly( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    this->calculate_x_axis( P0, P1 );

    this->for_polyhedral_mesh( P0, P1 );
}

void PlotOverLine::calculate_x_axis( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    const size_t resolution = m_x_axis.size();
    const float line_length = (P1-P0).length();
    const float offset = line_length / (resolution-1);
    float sampling_length = 0.0;

    for( int i=0; i<resolution; i++ )
    {
        m_x_axis[i] = sampling_length;
        sampling_length += offset;
    }
}

void PlotOverLine::for_structured_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    const size_t res_x = m_structured_volume->resolution().x();
    const size_t res_y = m_structured_volume->resolution().y();
    const size_t res_z = m_structured_volume->resolution().z();
    const size_t nx = res_x-1;
    const size_t ny = res_y-1;
    const size_t nz = res_z-1;
    const size_t ncoord = res_x*res_y*res_z;


    auto index = [res_x, res_y]( size_t I, size_t J, size_t K ){ return I + J*res_x + K*res_x*res_y; };

    const vismodule::UInt32 face_id[24] = {
        0, 1, 5, 4,
        1, 2, 6, 5,
        2, 3, 7, 6,
        3, 0, 4, 7,
        4, 5, 6, 7,
        3, 2, 1, 0 };

    for( size_t K = 0; K < nz; K++ )
    {
        for( size_t J = 0; J < ny; J++ )
        {
            for( size_t I = 0; I < nx; I++ )
            {

                vismodule::Vec3 vertex[8];
                vertex[0].set( I  , J+1, K+1 );
                vertex[1].set( I  , J  , K+1 );
                vertex[2].set( I+1, J  , K+1 );
                vertex[3].set( I+1, J+1, K+1 );
                vertex[4].set( I  , J+1, K   );
                vertex[5].set( I  , J  , K   );
                vertex[6].set( I+1, J  , K   );
                vertex[7].set( I+1, J+1, K   );
        

                // Bounding Box
                vismodule::Vec3 MinCoord( vertex[5] );
                vismodule::Vec3 MaxCoord( vertex[3] );
                if( this->intersection_of_boundingbox( MinCoord, MaxCoord, P0, P1 ) == false )
                {
                    continue;
                }

                vismodule::Vec3 face_center_vertex[6];
                size_t fid = 0;
                for( int i=0; i<6; i++, fid+=4 )
                {
                    face_center_vertex[i] = (
                        vertex[ face_id[fid  ] ] + vertex[ face_id[fid+1] ] +
                        vertex[ face_id[fid+2] ] + vertex[ face_id[fid+3] ]  ) * 0.25;
                }
                vismodule::Vec3 cell_center_vertex( I+0.5f, J+0.5f, K+0.5f );

                vismodule::Real32 scalar[8];
//                scalar[0] = m_structured_volume->values()[ index( I  , J+1, K+1 ) ].to<vismodule::Real32>();
//                scalar[1] = m_structured_volume->values()[ index( I  , J  , K+1 ) ].to<vismodule::Real32>();
//                scalar[2] = m_structured_volume->values()[ index( I+1, J  , K+1 ) ].to<vismodule::Real32>();
//                scalar[3] = m_structured_volume->values()[ index( I+1, J+1, K+1 ) ].to<vismodule::Real32>();
//                scalar[4] = m_structured_volume->values()[ index( I  , J+1, K   ) ].to<vismodule::Real32>();
//                scalar[5] = m_structured_volume->values()[ index( I  , J  , K   ) ].to<vismodule::Real32>();
//                scalar[6] = m_structured_volume->values()[ index( I+1, J  , K   ) ].to<vismodule::Real32>();
//                scalar[7] = m_structured_volume->values()[ index( I+1, J+1, K   ) ].to<vismodule::Real32>();   

                scalar[0] = m_structured_volume->values().to<vismodule::Real32>(index( I  , J+1, K+1 ) + ncoord*m_plot_variable);
                scalar[1] = m_structured_volume->values().to<vismodule::Real32>(index( I  , J  , K+1 ) + ncoord*m_plot_variable);
                scalar[2] = m_structured_volume->values().to<vismodule::Real32>(index( I+1, J  , K+1 ) + ncoord*m_plot_variable);
                scalar[3] = m_structured_volume->values().to<vismodule::Real32>(index( I+1, J+1, K+1 ) + ncoord*m_plot_variable);
                scalar[4] = m_structured_volume->values().to<vismodule::Real32>(index( I  , J+1, K   ) + ncoord*m_plot_variable);
                scalar[5] = m_structured_volume->values().to<vismodule::Real32>(index( I  , J  , K   ) + ncoord*m_plot_variable);
                scalar[6] = m_structured_volume->values().to<vismodule::Real32>(index( I+1, J  , K   ) + ncoord*m_plot_variable);
                scalar[7] = m_structured_volume->values().to<vismodule::Real32>(index( I+1, J+1, K   ) + ncoord*m_plot_variable);   

                vismodule::Real32 face_center_scalar[6];
                fid = 0;
                for( int i=0; i<6; i++, fid+=4 )
                {
                    face_center_scalar[i] = (
                        scalar[ face_id[fid  ] ] + scalar[ face_id[fid+1] ] +
                        scalar[ face_id[fid+2] ] + scalar[ face_id[fid+3] ]  ) * 0.25;
                }

                vismodule::Real32 cell_center_scalar = ((scalar[0]+scalar[1]+scalar[2]+scalar[3]+
                                                   scalar[4]+scalar[5]+scalar[6]+scalar[7] )*0.125 ); 

                vismodule::Vec3 vert[4];
                vismodule::Vec4 s;

                for( int face=0; face<6; face++ )
                {
                    for( int i=0; i<4; i++ )
                    {
                        //Face  0, 1, 5, 4
                        vert[0] = cell_center_vertex;
                        vert[1] = face_center_vertex[ face ];
                        vert[2] = vertex[ face_id[ face*4 + i ] ];
                        vert[3] = vertex[ face_id[ face*4 + ((i+1) % 4) ]];

                        s[0] = cell_center_scalar;
                        s[1] = face_center_scalar[ face ];
                        s[2] = scalar[ face_id[ face*4 + i ] ];
                        s[3] = scalar[ face_id[ face*4 + ((i+1) % 4) ] ];

                        this->sampling_in_tetrahedra( P0, P1, vert, s );
                    }
                }
            }
        }
    }
};

void PlotOverLine::for_tetrahedral_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    const vismodule::Real32* coords = m_volume->coords().pointer();
    const vismodule::UInt32* connec = m_volume->connections().pointer();
    const size_t ncells = m_volume->ncells();
    const size_t ncoord = m_volume->nnodes();

    size_t id = 0;

    for( int cell = 0; cell < ncells; cell++, id+=4 )
    {
        size_t local_id[4];
        local_id[0] = connec[ id   ];
        local_id[1] = connec[ id+1 ];
        local_id[2] = connec[ id+2 ];
        local_id[3] = connec[ id+3 ];

        vismodule::Vec3 vertex[4];
        vertex[0].set( coords[ local_id[0]*3 ], coords[ local_id[0]*3+1 ], coords[ local_id[0]*3+2 ] );
        vertex[1].set( coords[ local_id[1]*3 ], coords[ local_id[1]*3+1 ], coords[ local_id[1]*3+2 ] );
        vertex[2].set( coords[ local_id[2]*3 ], coords[ local_id[2]*3+1 ], coords[ local_id[2]*3+2 ] );
        vertex[3].set( coords[ local_id[3]*3 ], coords[ local_id[3]*3+1 ], coords[ local_id[3]*3+2 ] );

        vismodule::Vec4 scalar(
            m_volume->values().to<vismodule::Real32>(local_id[0] + ncoord*m_plot_variable),
            m_volume->values().to<vismodule::Real32>(local_id[1] + ncoord*m_plot_variable),
            m_volume->values().to<vismodule::Real32>(local_id[2] + ncoord*m_plot_variable),
            m_volume->values().to<vismodule::Real32>(local_id[3] + ncoord*m_plot_variable) );
            //m_volume->values()[ local_id[0] ].to<vismodule::Real32>(),
            //m_volume->values()[ local_id[1] ].to<vismodule::Real32>(),
            //m_volume->values()[ local_id[2] ].to<vismodule::Real32>(),
            //m_volume->values()[ local_id[3] ].to<vismodule::Real32>() );

        this->sampling_in_tetrahedra( P0, P1, vertex, scalar );
    }
}

void PlotOverLine::for_hexahedral_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    const vismodule::Real32* coords = m_volume->coords().pointer();
    const vismodule::UInt32* connec = m_volume->connections().pointer();
    const size_t ncells = m_volume->ncells();
    const size_t ncoord = m_volume->nnodes();

    const vismodule::UInt32 face_id[24] = {
        0, 1, 5, 4,
        1, 2, 6, 5,
        2, 3, 7, 6,
        3, 0, 4, 7,
        4, 5, 6, 7,
        3, 2, 1, 0 };

    size_t id = 0;

    for( int cell = 0; cell < ncells; cell++, id+=8 )
    {
        const vismodule::UInt32* local_id = &connec[ id ];

        vismodule::Vec3 vertex[8];
        for( int i=0; i<8; i++ )
        {
            vertex[i].set( coords[ local_id[i]*3 ], coords[ local_id[i]*3+1 ], coords[ local_id[i]*3+2 ] );
        }

        // Bounding Box
        vismodule::Vec3 MinCoord( vertex[0] );
        vismodule::Vec3 MaxCoord( vertex[0] );
        for( int i=1; i<8; i++ )
        {
            MinCoord.x() = MinCoord.x() < vertex[i].x() ? MinCoord.x() : vertex[i].x();
            MinCoord.y() = MinCoord.y() < vertex[i].y() ? MinCoord.y() : vertex[i].y();
            MinCoord.z() = MinCoord.z() < vertex[i].z() ? MinCoord.z() : vertex[i].z();
            MaxCoord.x() = MaxCoord.x() > vertex[i].x() ? MaxCoord.x() : vertex[i].x();
            MaxCoord.y() = MaxCoord.y() > vertex[i].y() ? MaxCoord.y() : vertex[i].y();
            MaxCoord.z() = MaxCoord.z() > vertex[i].z() ? MaxCoord.z() : vertex[i].z();
        }
        if( this->intersection_of_boundingbox( MinCoord, MaxCoord, P0, P1 ) == false )
        {
            continue;
        }

        vismodule::Vec3 face_center_vertex[6];
        size_t fid = 0;
        for( int i=0; i<6; i++, fid+=4 )
        {
            face_center_vertex[i] = (
                vertex[ face_id[fid  ] ] + vertex[ face_id[fid+1] ] +
                vertex[ face_id[fid+2] ] + vertex[ face_id[fid+3] ]  ) * 0.25;
        }

        vismodule::Vec3 cell_center_vertex( (vertex[0]+vertex[1]+vertex[2]+vertex[3]+
                                       vertex[4]+vertex[5]+vertex[6]+vertex[7] )*0.125 ); 

        vismodule::Real32 scalar[8];
        for( int i=0; i<8; i++ )
        {
            scalar[i] =  m_volume->values().to<vismodule::Real32>( local_id[i] + ncoord*m_plot_variable);
        }

        vismodule::Real32 face_center_scalar[6];
        fid = 0;
        for( int i=0; i<6; i++, fid+=4 )
        {
            face_center_scalar[i] = (
                scalar[ face_id[fid  ] ] + scalar[ face_id[fid+1] ] +
                scalar[ face_id[fid+2] ] + scalar[ face_id[fid+3] ]  ) * 0.25;
        }

        vismodule::Real32 cell_center_scalar = ((scalar[0]+scalar[1]+scalar[2]+scalar[3]+
                                           scalar[4]+scalar[5]+scalar[6]+scalar[7] )*0.125 ); 

        vismodule::Vec3 vert[4];
        vismodule::Vec4 s;

        for( int face=0; face<6; face++ )
        {
            for( int i=0; i<4; i++ )
            {
                //Face  0, 1, 5, 4
                vert[0] = cell_center_vertex;
                vert[1] = face_center_vertex[ face ];
                vert[2] = vertex[ face_id[ face*4 + i ] ];
                vert[3] = vertex[ face_id[ face*4 + ((i+1) % 4) ]];

                s[0] = cell_center_scalar;
                s[1] = face_center_scalar[ face ];
                s[2] = scalar[ face_id[ face*4 + i ] ];
                s[3] = scalar[ face_id[ face*4 + ((i+1) % 4) ] ];

                this->sampling_in_tetrahedra( P0, P1, vert, s );
            }
        }
    }
};

void PlotOverLine::for_pyramidal_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    const vismodule::Real32* coords = m_volume->coords().pointer();
    const vismodule::UInt32* connec = m_volume->connections().pointer();
    const size_t ncells = m_volume->ncells();
    const size_t ncoord = m_volume->nnodes();

    // for AVS
    const vismodule::UInt32 face_id[24] = { 1, 2, 3, 4 };

    // for VTK
    //const vismodule::UInt32 face_id[24] = { 3, 2, 1, 0 };

    size_t id = 0;

    for( int cell = 0; cell < ncells; cell++, id+=5 )
    {
        const vismodule::UInt32* local_id = &connec[ id ];

        vismodule::Vec3 vertex[5];
        for( int i=0; i<5; i++ )
        {
            vertex[i].set( coords[ local_id[i]*3 ], coords[ local_id[i]*3+1 ], coords[ local_id[i]*3+2 ] );
        }

        // Bounding Box
        vismodule::Vec3 MinCoord( vertex[0] );
        vismodule::Vec3 MaxCoord( vertex[0] );
        for( int i=1; i<5; i++ )
        {
            MinCoord.x() = MinCoord.x() < vertex[i].x() ? MinCoord.x() : vertex[i].x();
            MinCoord.y() = MinCoord.y() < vertex[i].y() ? MinCoord.y() : vertex[i].y();
            MinCoord.z() = MinCoord.z() < vertex[i].z() ? MinCoord.z() : vertex[i].z();
            MaxCoord.x() = MaxCoord.x() > vertex[i].x() ? MaxCoord.x() : vertex[i].x();
            MaxCoord.y() = MaxCoord.y() > vertex[i].y() ? MaxCoord.y() : vertex[i].y();
            MaxCoord.z() = MaxCoord.z() > vertex[i].z() ? MaxCoord.z() : vertex[i].z();
        }
        if( this->intersection_of_boundingbox( MinCoord, MaxCoord, P0, P1 ) == false )
        {
            continue;
        }

        vismodule::Vec3 face_center_vertex[1];
        size_t fid = 0;
        for( int i=0; i<1; i++, fid+=4 )
        {
            face_center_vertex[i] = (
                vertex[ face_id[fid  ] ] + vertex[ face_id[fid+1] ] +
                vertex[ face_id[fid+2] ] + vertex[ face_id[fid+3] ]  ) * 0.25;
        }

        // for AVS
        vismodule::Vec3 cell_center_vertex( vertex[0] );

        // for VTK
        //vismodule::Vec3 cell_center_vertex( vertex[4] );

        vismodule::Real32 scalar[5];
        for( int i=0; i<5; i++ )
        {
            scalar[i] =  m_volume->values().to<vismodule::Real32>(local_id[i] + ncoord*m_plot_variable );
        }

        vismodule::Real32 face_center_scalar[1];
        fid = 0;
        for( int i=0; i<1; i++, fid+=4 )
        {
            face_center_scalar[i] = (
                scalar[ face_id[fid  ] ] + scalar[ face_id[fid+1] ] +
                scalar[ face_id[fid+2] ] + scalar[ face_id[fid+3] ]  ) * 0.25;
        }

        // for AVS
        vismodule::Real32 cell_center_scalar = ( scalar[0] );

        //for VTK
        //vismodule::Real32 cell_center_scalar = ( scalar[4] ); 

        vismodule::Vec3 vert[4];
        vismodule::Vec4 s;

        for( int face=0; face<1; face++ )
        {
            for( int i=0; i<4; i++ )
            {
                vert[0] = cell_center_vertex;
                vert[1] = face_center_vertex[ face ];
                vert[2] = vertex[ face_id[ face*4 + i ] ];
                vert[3] = vertex[ face_id[ face*4 + ((i+1) % 4) ]];

                s[0] = cell_center_scalar;
                s[1] = face_center_scalar[ face ];
                s[2] = scalar[ face_id[ face*4 + i ] ];
                s[3] = scalar[ face_id[ face*4 + ((i+1) % 4) ] ];

                this->sampling_in_tetrahedra( P0, P1, vert, s );
            }
        }
    }
};

void PlotOverLine::for_prismic_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    const vismodule::Real32* coords = m_volume->coords().pointer();
    const vismodule::UInt32* connec = m_volume->connections().pointer();
    const size_t ncells = m_volume->ncells();
    const size_t ncoord = m_volume->nnodes();

    const vismodule::UInt32 face_id[12] = {
        0, 1, 4, 3,
        1, 2, 5, 4,
        2, 0, 3, 5 };

    size_t id = 0;

    for( int cell = 0; cell < ncells; cell++, id+=6 )
    {
        const vismodule::UInt32* local_id = &connec[ id ];

        vismodule::Vec3 vertex[6];
        for( int i=0; i<6; i++ )
        {
            vertex[i].set( coords[ local_id[i]*3 ], coords[ local_id[i]*3+1 ], coords[ local_id[i]*3+2 ] );
        }

        // Bounding Box
        vismodule::Vec3 MinCoord( vertex[0] );
        vismodule::Vec3 MaxCoord( vertex[0] );
        for( int i=1; i<6; i++ )
        {
            MinCoord.x() = MinCoord.x() < vertex[i].x() ? MinCoord.x() : vertex[i].x();
            MinCoord.y() = MinCoord.y() < vertex[i].y() ? MinCoord.y() : vertex[i].y();
            MinCoord.z() = MinCoord.z() < vertex[i].z() ? MinCoord.z() : vertex[i].z();
            MaxCoord.x() = MaxCoord.x() > vertex[i].x() ? MaxCoord.x() : vertex[i].x();
            MaxCoord.y() = MaxCoord.y() > vertex[i].y() ? MaxCoord.y() : vertex[i].y();
            MaxCoord.z() = MaxCoord.z() > vertex[i].z() ? MaxCoord.z() : vertex[i].z();
        }
        if( this->intersection_of_boundingbox( MinCoord, MaxCoord, P0, P1 ) == false )
        {
            continue;
        }

        // Square to triangle
        vismodule::Vec3 face_center_vertex[3];
        size_t fid = 0;
        for( int i=0; i<3; i++, fid+=4 )
        {
            face_center_vertex[i] = (
                vertex[ face_id[fid  ] ] + vertex[ face_id[fid+1] ] +
                vertex[ face_id[fid+2] ] + vertex[ face_id[fid+3] ]  ) * 0.25;
        }

        vismodule::Vec3 cell_center_vertex( (vertex[0]+vertex[1]+vertex[2]+
                                       vertex[3]+vertex[4]+vertex[5] )/6.0 ); 

        vismodule::Real32 scalar[6];
        for( int i=0; i<6; i++ )
        {
            scalar[i] =  m_volume->values().to<vismodule::Real32>(local_id[i] + ncoord*m_plot_variable);
        }

        vismodule::Real32 face_center_scalar[3];
        fid = 0;
        for( int i=0; i<3; i++, fid+=4 )
        {
            face_center_scalar[i] = (
                scalar[ face_id[fid  ] ] + scalar[ face_id[fid+1] ] +
                scalar[ face_id[fid+2] ] + scalar[ face_id[fid+3] ]  ) * 0.25;
        }

        vismodule::Real32 cell_center_scalar = ((scalar[0]+scalar[1]+scalar[2]+
                                           scalar[3]+scalar[4]+scalar[5] )/6.0 ); 

        vismodule::Vec3 vert[4];
        vismodule::Vec4 s;

        for( int face=0; face<3; face++ )
        {
            for( int i=0; i<4; i++ )
            {
                vert[0] = cell_center_vertex;
                vert[1] = face_center_vertex[ face ];
                vert[2] = vertex[ face_id[ face*4 + i ] ];
                vert[3] = vertex[ face_id[ face*4 + ((i+1) % 4) ]];

                s[0] = cell_center_scalar;
                s[1] = face_center_scalar[ face ];
                s[2] = scalar[ face_id[ face*4 + i ] ];
                s[3] = scalar[ face_id[ face*4 + ((i+1) % 4) ] ];

                this->sampling_in_tetrahedra( P0, P1, vert, s );
            }
        }

        // Upper triangle
        vert[0] = cell_center_vertex;
        vert[1] = vertex[2];
        vert[2] = vertex[1];
        vert[3] = vertex[0];

        s[0] = cell_center_scalar;
        s[1] = scalar[2];
        s[2] = scalar[1];
        s[3] = scalar[0];

        this->sampling_in_tetrahedra( P0, P1, vert, s );

        // Lower triangle
        vert[0] = cell_center_vertex;
        vert[1] = vertex[3];
        vert[2] = vertex[4];
        vert[3] = vertex[5];

        s[0] = cell_center_scalar;
        s[1] = scalar[3];
        s[2] = scalar[4];
        s[3] = scalar[5];

        this->sampling_in_tetrahedra( P0, P1, vert, s );
    } // end of for cell
};

void PlotOverLine::for_polyhedral_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    const vismodule::Real32* coords = m_polyhedron->coords.pointer();
    const vismodule::UInt32* connec = m_polyhedron->connections.pointer();
    const vismodule::UInt32* conoff = m_polyhedron->connection_offsets.pointer();
    const vismodule::UInt32* faces  = m_polyhedron->faces.pointer();
    const vismodule::UInt32* facoff = m_polyhedron->face_offsets.pointer();
    const vismodule::Real32* scalar = m_polyhedron->scalars.pointer();
    const size_t ncells = m_polyhedron->ncells;

    for( int cell = 0; cell < ncells; cell++ )
    {
        const size_t vid0 = cell==0 ? 0 : conoff[ cell -1 ];
        const size_t vid1 = conoff[ cell ];

        // Bounding Box
        const size_t id0 = connec[vid0]*3;
        const vismodule::Vec3 vertex0( coords[id0],coords[id0+1],coords[id0+2] );
        vismodule::Vec3 MinCoord( vertex0 );
        vismodule::Vec3 MaxCoord( vertex0 );
        for( int i=vid0+1; i<vid1; i++ )
        {
            const size_t id = connec[i]*3;
            const vismodule::Vec3 vertex( coords[id], coords[id+1], coords[id+2] );
            MinCoord.x() = MinCoord.x() < vertex.x() ? MinCoord.x() : vertex.x();
            MinCoord.y() = MinCoord.y() < vertex.y() ? MinCoord.y() : vertex.y();
            MinCoord.z() = MinCoord.z() < vertex.z() ? MinCoord.z() : vertex.z();
            MaxCoord.x() = MaxCoord.x() > vertex.x() ? MaxCoord.x() : vertex.x();
            MaxCoord.y() = MaxCoord.y() > vertex.y() ? MaxCoord.y() : vertex.y();
            MaxCoord.z() = MaxCoord.z() > vertex.z() ? MaxCoord.z() : vertex.z();
        }
        if( this->intersection_of_boundingbox( MinCoord, MaxCoord, P0, P1 ) == false )
        {
            continue;
        }

        // Cell center vertex
        vismodule::Vec3 cell_center_vertex( vertex0 );
        for( int i=vid0+1; i<vid1; i++ )
        {
            const size_t id = connec[i]*3;
            cell_center_vertex += vismodule::Vec3( coords[id], coords[id+1], coords[id+2] );
        }
        const size_t ncellvert = vid1 - vid0;
        cell_center_vertex /= (vismodule::Real32)ncellvert;

        // Cell center scalar
        vismodule::Real32 cell_center_scalar = 0.0f;
        for( int i=vid0; i<vid1; i++ )
        {
            const size_t id = connec[i];
            cell_center_scalar += scalar[id];
        }
        cell_center_scalar /= (vismodule::Real32)ncellvert;

        // Access each face
        const size_t fid0 = cell==0 ? 0 : facoff[ cell -1 ];

        const vismodule::UInt32* pface_array = &faces[ fid0 ];
        const size_t nfaces = *pface_array;
        pface_array++;

        for( int fcnt=0; fcnt<nfaces; fcnt++ )
        {
            // Calc. Face center
            vismodule::Vec3 face_center_vertex( 0.0f, 0.0f, 0.0f );
            vismodule::Real32 face_center_scalar = 0.0f;
            const size_t nfacevert = *pface_array;
            pface_array++;

            // keep a pointer for the start of face array
            // It is used to travel face array for trianguration.
            const vismodule::UInt32* pface = pface_array;

            for( int vcnt=0; vcnt<nfacevert; vcnt++ )
            {
                const size_t face_vert_id = *pface_array;
                pface_array++;
                const vismodule::Vec3 vertex( coords[face_vert_id*3  ],
                                        coords[face_vert_id*3+1],
                                        coords[face_vert_id*3+2] );
                face_center_vertex += vertex;

                const vismodule::Real32 scal = scalar[face_vert_id];
                face_center_scalar += scal;
            }
            face_center_vertex /= (vismodule::Real32)nfacevert;
            face_center_scalar /= (vismodule::Real32)nfacevert;

            vismodule::Vec3 vert[4];
            vismodule::Vec4 s;
            for( int i=0; i<nfacevert; i++ )
            {
                const size_t vid2 = *(pface + i);
                const size_t vid3 = *(pface + (i+1)%nfacevert);
                vert[0] = cell_center_vertex;
                vert[1] = face_center_vertex;
                vert[2] = vismodule::Vec3( coords[vid2*3], coords[vid2*3+1], coords[vid2*3+2] );
                vert[3] = vismodule::Vec3( coords[vid3*3], coords[vid3*3+1], coords[vid3*3+2] );

                s[0] = cell_center_scalar;
                s[1] = face_center_scalar;
                s[2] = scalar[ vid2 ];
                s[3] = scalar[ vid3 ];

                this->sampling_in_tetrahedra( P0, P1, vert, s );
            }
        } // end of for each face
    } // end of for each cell
};

void PlotOverLine::sampling_in_tetrahedra( const vismodule::Vec3 P0, const vismodule::Vec3 P1,
                                           const vismodule::Vec3* vertices,
                                           const vismodule::Vec4 scalars )
{
    // Tetrahedra vertices and scalars
    const vismodule::Vec3 X0 = vertices[0];
    const vismodule::Vec3 X1 = vertices[1]; 
    const vismodule::Vec3 X2 = vertices[2];
    const vismodule::Vec3 X3 = vertices[3];
    const vismodule::Vec4 S( scalars );
    
    // Bounding Box
    vismodule::Vec3 MinCoord( vismodule::Math::Min<float>( X0.x(), X1.x(), X2.x(), X3.x() ),
                        vismodule::Math::Min<float>( X0.y(), X1.y(), X2.y(), X3.y() ),
                        vismodule::Math::Min<float>( X0.z(), X1.z(), X2.z(), X3.z() ) );
    vismodule::Vec3 MaxCoord( vismodule::Math::Max<float>( X0.x(), X1.x(), X2.x(), X3.x() ),
                        vismodule::Math::Max<float>( X0.y(), X1.y(), X2.y(), X3.y() ),
                        vismodule::Math::Max<float>( X0.z(), X1.z(), X2.z(), X3.z() ) );


    if( this->intersection_of_boundingbox( MinCoord, MaxCoord, P0, P1 ) == false )
    {
        //std::cout << "Line P doesn't intersect tet's bounding box." << std::endl;
        return;
    }

    // Barycentric coordinates of P0 := ( a0, b0, c0, d0 ).
    const vismodule::Vec4 bc_P0  =  this->barycentric_coordinates( X0, X1, X2, X3, P0 );

    // Barycentric coordinates of P1 := ( a1, b1, c1, d1 )
    const vismodule::Vec4 bc_P1  =  this->barycentric_coordinates( X0, X1, X2, X3, P1 );

    //std::cout << "range of t for each component of barycentric coordinates." << std::endl;
    POL::Range t_range = this->t_range_in_tet( bc_P0, bc_P1 );
    if( t_range.non_empty() == false )
    {
        //std::cout << __LINE__ << ": " << "P0 -> P1 line doesn't intersect tetrahedra." << std::endl;
        return;
    }

    // Extraction of sampling points in tet.
    const int nsamples = m_values_on_line.size();

    // Discretization of t range from [0,1] to [0,255]
    const int discretized_t_lower = vismodule::Math::Ceil <float>( t_range.lower() * (nsamples-1) );
    const int discretized_t_upper = vismodule::Math::Floor<float>( t_range.upper() * (nsamples-1) );

    for( int i = discretized_t_lower; i <= discretized_t_upper; i++ )
    {
        const float t = (float)i/(nsamples-1);
        const vismodule::Vec4 bc_P = bc_P0 + t * (bc_P1 - bc_P0);
        m_values_on_line[i] = bc_P.dot(S);
        m_mask[i] = true;
 
    }

    return;
}

const bool PlotOverLine::intersection_of_boundingbox(
    const vismodule::Vec3 MinCoord, const vismodule::Vec3 MaxCoord,
    const vismodule::Vec3 P0, const vismodule::Vec3 P1 )
{
    // P(t) = P0 + t (P1 - P0)
    float t_min = 0.0f;
    float t_max = 1.0f;

    for( int i=0; i<3; i++ )
    {
        const float delta = P1[i] - P0[i];

        if( vismodule::Math::Equal( P1[i], P0[i]) )
        {
            if( P0[i] < MinCoord[i] || MaxCoord[i] < P0[i] )
            {
                return false;
            }
        }
        else
        {
            float t1 = ( MinCoord[i] - P0[i] ) / delta;
            float t2 = ( MaxCoord[i] - P0[i] ) / delta;

            if( t1 > t2 ) std::swap( t1, t2 );

            t_min = (std::max)( t_min, t1 );
            t_max = (std::min)( t_max, t2 );

            if( t_min > t_max )
            {          
                return false;
            }
        }
    }

    return true;
}

// Tetrahedral volume is M.determinant/6.
// Because barycentric coordinated is ratio of tetrahedral volume, 1/6 is negligible.
const float PlotOverLine::tetrahedra_signed_volume_X6(
    const vismodule::Vec3 X0, const vismodule::Vec3 X1, const vismodule::Vec3 X2, const vismodule::Vec3 X3 )
{
    const vismodule::Vec3 X0_3 = X0 - X3;
    const vismodule::Vec3 X1_3 = X1 - X3;
    const vismodule::Vec3 X2_3 = X2 - X3;
    const vismodule::Mat3 M(
        X0_3.x(), X1_3.x(), X2_3.x(),
        X0_3.y(), X1_3.y(), X2_3.y(),
        X0_3.z(), X1_3.z(), X2_3.z() );

    return M.determinant();
}

// Barycentric coordinates := (a, b, c, d)
// Here, P = a X0 + b X1 + c X2 + d X3
// a+b+c+d=1 ( a=V0/V, b=V1/V,...  makes this condition true.)
// if a, b, c, d >= 0 , P is included in tetrahedra
// this method is free for the direction of tetrahedra.
const vismodule::Vec4 PlotOverLine::barycentric_coordinates(
    const vismodule::Vec3 X0, const vismodule::Vec3 X1, const vismodule::Vec3 X2, const vismodule::Vec3 X3, const vismodule::Vec3 P )
{
    const float V  = tetrahedra_signed_volume_X6( X0, X1, X2, X3 );
    const float V0 = tetrahedra_signed_volume_X6(  P, X1, X2, X3 );
    const float V1 = tetrahedra_signed_volume_X6( X0,  P, X2, X3 );
    const float V2 = tetrahedra_signed_volume_X6( X0, X1,  P, X3 );
    const float V3 = tetrahedra_signed_volume_X6( X0, X1, X2,  P );

    return vismodule::Vec4( V0/V, V1/V, V2/V, V3/V );
}

// Line segment parametric expression: P(t) = P0 + t (P1 - P0) , 0<=t<=1
// P(t) is expanded by barycentric coodinates of P1 and P2 as ( a(t), b(t), c(t), d(t) ).
// where
// a(t)  =  a0 + t (a1 - a0)
// b(t)  =  b0 + t (b1 - b0)
// c(t)  =  c0 + t (c1 - c0)
// d(t)  =  d0 + t (d1 - d0)
// P(t) is included if a(t), b(t), c(t), d(t) >= 0 with 0<=t<=1.
POL::Range PlotOverLine::t_range_in_tet( const vismodule::Vec4 bc_P0, const vismodule::Vec4 bc_P1 )
{
    POL::Range t_range(0,1);

    for( int i=0; i<4; i++)
    {
        POL::Range tmp_range;

        float a0 =  (float)bc_P0[i];
        float a1 =  (float)bc_P1[i];
        if( vismodule::Math::Abs(a0) < 1e-6 ) a0 = 0.f;
        if( vismodule::Math::Abs(a1) < 1e-6 ) a1 = 0.f;

        if( vismodule::Math::Equal( a0, a1 ) )
        {
            if( a0 >= 0.0 )
            {
                // any t makes a(t)>=0.
                tmp_range.set( 0.0, 1.0 );
            }
            else
            {
                // any t makes a(t)<0.
                return POL::Range( 3, -3 );
            }
        }
        else
        {
            const float det = -a0/(a1-a0);

            if( a1 - a0 > 0.0 )
            {
                if( det < 0.0 )
                {
                    tmp_range.set( 0.0, 1.0 );
                }
                else if( 0.0 <= det && det <= 1.0 )
                {
                    tmp_range.set( det, 1.0 );
                }
                else // (1.0 < det)
                {
                    return POL::Range( 4, -4 );
                }
            }
            else // ( a1 - a0 < 0.0 )
            {
                if( 1.0 < det)
                {
                    tmp_range.set( 0.0, 1.0 );
                }
                else if( 0.0 <= det && det <= 1.0 )
                {
                    tmp_range.set( 0.0, det );
                }
                else //( det < 0.0 )
                {
                    return POL::Range( 5, -5 );
                }

            }
        }
        //tmp_range.print();

        t_range = t_range.intersect( tmp_range );

        if( t_range.non_empty() == false )
        {
            return POL::Range( 6, -6 );
        }
    }

    return t_range;
}

void PlotOverLine::OutputLine( const int time_step)
{
    if (!m_plot_flag) return; 

    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank=0;
    mpi_size=1;
#endif
    int nbins = 256;

    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    vismodule::ValueArray<float> values_on_line( m_allcell_values_on_line  );
    vismodule::ValueArray<float> x_axis( m_x_axis );
    vismodule::ValueArray<bool>  mask ( m_allcell_mask   );
#if 0
    static bool first_step = true;
    static MPI_Comm new_comm;
    static int count;
    static int num_nodes;
#endif

    std::stringstream ss;
    //add by shimomura 20240614
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank+1;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".dat";
    m_POLFilePath += ss.str();
    // 20181226 end

    vismodule::KVSMLObjectPlotOverLine kvsmlobject( values_on_line, m_x_axis, m_mask);
    kvsmlobject.write(m_POLFilePath.c_str());

}


