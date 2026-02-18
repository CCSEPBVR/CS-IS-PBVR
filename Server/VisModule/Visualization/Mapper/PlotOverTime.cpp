#include <vismodule/PlotOverTime>

// structured
PlotOverTime::PlotOverTime(
    domain_parameters_struct dom,
    float** values,
    int nvariables, 
    const PlotOverTimeProperty& pot_property
):
m_values( values ),
m_nvariables( nvariables ),
m_dom( dom )
{
    vismodule::Vec3 P(
        pot_property.m_target_point[0],
        pot_property.m_target_point[1],
        pot_property.m_target_point[2]
    );

    std::cout << "Target Point = ";
    std::cout << P.x() << ", ";
    std::cout << P.y() << ", ";
    std::cout << P.z() << std::endl;

    m_values_on_time.allocate( m_nvariables );
    m_values_on_time.fill( 0x00 );

    m_mask = false;

    this->for_structured_mesh( P );
}

// unstructured
PlotOverTime::PlotOverTime(
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const  vismodule::VolumeObjectBase::CellType& celltype, 
    const PlotOverTimeProperty& pot_property
):
m_values( values ),
m_nvariables( nvariables ),
m_coordinates( coordinates ),
m_nnodes( ncoords ),
m_connections( connections ),
m_ncells( ncells ),
m_cellType( celltype )
{
    vismodule::Vec3 P(
        pot_property.m_target_point[0],
        pot_property.m_target_point[1],
        pot_property.m_target_point[2]
    );

    std::cout << "Target Point = ";
    std::cout << P.x() << ", ";
    std::cout << P.y() << ", ";
    std::cout << P.z() << std::endl;

    m_values_on_time.allocate( m_nvariables );
    m_values_on_time.fill( 0x00 );

    m_mask = false;

    switch ( m_cellType )
    {
        case vismodule::UnstructuredVolumeObject::Tetrahedra:
        {
            this->for_tetrahedral_mesh( P );
            break;
        }
        case vismodule::UnstructuredVolumeObject::Hexahedra:
        {
            this->for_hexahedral_mesh( P );
            break;
        }
        case vismodule::UnstructuredVolumeObject::Pyramid:
        {
            this->for_pyramidal_mesh( P );
            break;
        }
        case vismodule::UnstructuredVolumeObject::Prism:
        {
            this->for_prismic_mesh( P );
            break;
        }
        default:
        {
            std::cout << "nonavailable  celltype!!" << std::endl;
            break;
        }
    }
}

void PlotOverTime::for_structured_mesh( const vismodule::Vec3 P )
{
    const size_t res_x = m_dom.resolution[0];
    const size_t res_y = m_dom.resolution[1];
    const size_t res_z = m_dom.resolution[2];
    const size_t nx = res_x-1;
    const size_t ny = res_y-1;
    const size_t nz = res_z-1;
    const size_t ncoord     = res_x*res_y*res_z;
    const size_t nvariables = m_nvariables;

    auto index = [res_x, res_y]( size_t I, size_t J, size_t K ){ return I + J*res_x + K*res_x*res_y; };

    const vismodule::UInt32 face_id[24] = {
        0, 1, 5, 4,
        1, 2, 6, 5,
        2, 3, 7, 6,
        3, 0, 4, 7,
        4, 5, 6, 7,
        3, 2, 1, 0 };

    // min of each process
    const float x_min = m_dom.x_min;
    const float y_min = m_dom.y_min;
    const float z_min = m_dom.z_min;
    const float cell_length = 1;

    for( size_t K = 0; K < nz; K++ )
    {
        for( size_t J = 0; J < ny; J++ )
        {
            for( size_t I = 0; I < nx; I++ )
            {

                vismodule::Vec3 vertex[8];
                vertex[0].set( float(I    ) * cell_length + x_min  , float(J + 1) * cell_length + y_min, float(K + 1) * cell_length + z_min );
                vertex[1].set( float(I    ) * cell_length + x_min  , float(J    ) * cell_length + y_min, float(K + 1) * cell_length + z_min );
                vertex[2].set( float(I + 1) * cell_length + x_min  , float(J    ) * cell_length + y_min, float(K + 1) * cell_length + z_min );
                vertex[3].set( float(I + 1) * cell_length + x_min  , float(J + 1) * cell_length + y_min, float(K + 1) * cell_length + z_min );
                vertex[4].set( float(I    ) * cell_length + x_min  , float(J + 1) * cell_length + y_min, float(K    ) * cell_length + z_min );
                vertex[5].set( float(I    ) * cell_length + x_min  , float(J    ) * cell_length + y_min, float(K    ) * cell_length + z_min );
                vertex[6].set( float(I + 1) * cell_length + x_min  , float(J    ) * cell_length + y_min, float(K    ) * cell_length + z_min );
                vertex[7].set( float(I + 1) * cell_length + x_min  , float(J + 1) * cell_length + y_min, float(K    ) * cell_length + z_min );
        
                // Bounding Box
                vismodule::Vec3 MinCoord( vertex[5] );
                vismodule::Vec3 MaxCoord( vertex[3] );
                if( this->is_point_in_boundingbox( MinCoord, MaxCoord, P ) == false )
                {
                    continue;
                }

                vismodule::Vec3 face_center_vertex[6];
                size_t fid = 0;
                for( size_t i = 0; i < 6; i++, fid += 4 )
                {
                    face_center_vertex[i] = (
                        vertex[face_id[fid    ]] + vertex[face_id[fid + 1]] +
                        vertex[face_id[fid + 2]] + vertex[face_id[fid + 3]]
                    ) * 0.25;
                }
                vismodule::Vec3 cell_center_vertex((
                    vertex[0] + vertex[1] + vertex[2] + vertex[3] +
                    vertex[4] + vertex[5] + vertex[6] + vertex[7] ) * 0.125 ); 

                vismodule::Real32** struct_scalars;
                vismodule::Real32** face_center_scalars;
                vismodule::Real32*  cell_center_scalars;
                struct_scalars      = new vismodule::Real32*[nvariables];
                face_center_scalars = new vismodule ::Real32*[nvariables];
                cell_center_scalars = new vismodule::Real32[nvariables];

                for ( size_t i = 0; i < nvariables; i++ )
                {
                    struct_scalars[i] = new vismodule::Real32[8];
                    struct_scalars[i][0] = m_values[i][index( I    , J + 1, K + 1 )];
                    struct_scalars[i][1] = m_values[i][index( I    , J    , K + 1 )];
                    struct_scalars[i][2] = m_values[i][index( I + 1, J    , K + 1 )];
                    struct_scalars[i][3] = m_values[i][index( I + 1, J + 1, K + 1 )];
                    struct_scalars[i][4] = m_values[i][index( I    , J + 1, K     )];
                    struct_scalars[i][5] = m_values[i][index( I    , J    , K     )];
                    struct_scalars[i][6] = m_values[i][index( I + 1, J    , K     )];
                    struct_scalars[i][7] = m_values[i][index( I + 1, J + 1, K     )];

                    face_center_scalars[i] = new vismodule::Real32[6];
                    fid = 0;
                    for( int j = 0; j < 6; j++, fid += 4 )
                    {
                        face_center_scalars[i][j] = (
                            struct_scalars[i][face_id[fid    ]] + struct_scalars[i][face_id[fid + 1]] +
                            struct_scalars[i][face_id[fid + 2]] + struct_scalars[i][face_id[fid + 3]]
                        ) * 0.25;
                    }

                    cell_center_scalars[i] = ((
                        struct_scalars[i][0] + struct_scalars[i][1] + struct_scalars[i][2] + struct_scalars[i][3] +
                        struct_scalars[i][4] + struct_scalars[i][5] + struct_scalars[i][6] + struct_scalars[i][7]
                        ) * 0.125 );
                }

                vismodule::Vec3  vert[4];
                vismodule::Vec4* tetra_scalars;
                tetra_scalars = new vismodule::Vec4[nvariables];

                for( size_t face = 0; face < 6; face++ )
                {
                    for( size_t i = 0; i < 4; i++ )
                    {
                        //Face  0, 1, 5, 4
                        vert[0] = cell_center_vertex;
                        vert[1] = face_center_vertex[face];
                        vert[2] = vertex[face_id[face*4 + i]];
                        vert[3] = vertex[face_id[face*4 + ((i + 1) % 4)]];

                        for ( size_t j = 0; j < nvariables; j++ )
                        {
                            tetra_scalars[j][0] = cell_center_scalars[j];
                            tetra_scalars[j][1] = face_center_scalars[j][face];
                            tetra_scalars[j][2] = struct_scalars[j][face_id[face*4 + i ]];
                            tetra_scalars[j][3] = struct_scalars[j][face_id[face*4 + ((i + 1) % 4)]];
                        }

                        this->sampling_in_tetrahedra( P, vert, tetra_scalars, nvariables );
                        if ( m_mask ) break;
                    }
                    if ( m_mask ) break;
                }

                for( size_t i = 0; i < nvariables; i++ )
                {
                    delete[] struct_scalars[i];
                    delete[] face_center_scalars[i];
                }

                delete[] struct_scalars;
                delete[] face_center_scalars;
                delete[] cell_center_scalars;
                delete[] tetra_scalars;

                if ( m_mask ) break;
            }
        }
    }
}

void PlotOverTime::for_tetrahedral_mesh( const vismodule::Vec3 P )
{
    const vismodule::Real32* coords = m_coordinates;
    const vismodule::UInt32* connec = m_connections;
    const size_t ncells     = m_ncells;
    const size_t ncoord     = m_nnodes;
    const size_t nvariables = m_nvariables;

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

        vismodule::Vec4* scalars;
        scalars = new vismodule::Vec4[nvariables];

        for ( int i = 0; i < nvariables; i++ )
        {
            scalars[i][0] = m_values[i][ local_id[0] ];
            scalars[i][1] = m_values[i][ local_id[1] ];
            scalars[i][2] = m_values[i][ local_id[2] ];
            scalars[i][3] = m_values[i][ local_id[3] ];
        }

        this->sampling_in_tetrahedra( P, vertex, scalars, nvariables );

        delete[] scalars;

        if ( m_mask ) break;
    }
}

void PlotOverTime::for_hexahedral_mesh( const vismodule::Vec3 P )
{

    const vismodule::Real32*  coords = m_coordinates;
    const vismodule::UInt32*  connec = m_connections;
    const size_t ncells     = m_ncells;
    const size_t ncoord     = m_nnodes;
    const size_t nvariables = m_nvariables;

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

        if( this->is_point_in_boundingbox( MinCoord, MaxCoord, P ) == false )
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

        vismodule::Real32** hexa_scalars;
        vismodule::Real32** face_center_scalars;
        vismodule::Real32*  cell_center_scalars;
        hexa_scalars        = new vismodule::Real32*[ nvariables ];
        face_center_scalars = new vismodule::Real32*[ nvariables ];
        cell_center_scalars = new vismodule::Real32[ nvariables ];

        for( int i=0; i<nvariables; i++ )
        {
            hexa_scalars[i] = new vismodule::Real32[8];
            for( int j=0; j<8; j++ )
            {
                hexa_scalars[i][j] = m_values[i][ local_id[j] ];
            }

            face_center_scalars[i] = new vismodule::Real32[6];
            fid = 0;
            for( int j=0; j<6; j++, fid+=4 )
            {
                face_center_scalars[i][j] = (
                    hexa_scalars[i][ face_id[fid  ] ] + hexa_scalars[i][ face_id[fid+1] ] +
                    hexa_scalars[i][ face_id[fid+2] ] + hexa_scalars[i][ face_id[fid+3] ]  ) * 0.25;
            }

            cell_center_scalars[i] = ((
                hexa_scalars[i][0]+hexa_scalars[i][1]+hexa_scalars[i][2]+hexa_scalars[i][3]+
                hexa_scalars[i][4]+hexa_scalars[i][5]+hexa_scalars[i][6]+hexa_scalars[i][7]) * 0.125 );
        }

        vismodule::Vec3  vert[4];
        vismodule::Vec4* tetra_scalars;
        tetra_scalars = new vismodule::Vec4[nvariables];

        for( int face=0; face<6; face++ )
        {
            for( int i=0; i<4; i++ )
            {
                //Face  0, 1, 5, 4
                vert[0] = cell_center_vertex;
                vert[1] = face_center_vertex[ face ];
                vert[2] = vertex[ face_id[ face*4 + i ] ];
                vert[3] = vertex[ face_id[ face*4 + ((i+1) % 4) ]];

                for( int j=0; j<nvariables; j++ )
                {
                    tetra_scalars[j][0] = cell_center_scalars[j];
                    tetra_scalars[j][1] = face_center_scalars[j][ face ];
                    tetra_scalars[j][2] = hexa_scalars[j][ face_id[ face*4 + i ] ];
                    tetra_scalars[j][3] = hexa_scalars[j][ face_id[ face*4 + ((i+1) % 4) ] ];
                }

                this->sampling_in_tetrahedra( P, vert, tetra_scalars, nvariables );
                if ( m_mask ) break;
            }
            if ( m_mask ) break;
        }

        for( int i=0; i<nvariables; i++ )
        {
            delete[] hexa_scalars[i];
            delete[] face_center_scalars[i];
        }

        delete[] hexa_scalars;
        delete[] face_center_scalars;
        delete[] cell_center_scalars;
        delete[] tetra_scalars;

        if ( m_mask ) break;
    }
};

void PlotOverTime::for_pyramidal_mesh( const vismodule::Vec3 P )
{
    const vismodule::Real32* coords = m_coordinates;
    const vismodule::UInt32* connec = m_connections;
    const size_t ncells     = m_ncells;
    const size_t ncoord     = m_nnodes;
    const size_t nvariables = m_nvariables;

    // for AVS
    const vismodule::UInt32 face_id[24] = { 1, 2, 3, 4 };

    // for VTK
    // const vismodule::UInt32 face_id[24] = { 3, 2, 1, 0 };

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
        if( this->is_point_in_boundingbox( MinCoord, MaxCoord, P ) == false )
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
        // vismodule::Vec3 cell_center_vertex( vertex[4] );

        vismodule::Real32** pyramidal_scalars;
        vismodule::Real32** face_center_scalars;
        vismodule::Real32*  cell_center_scalars;
        pyramidal_scalars   = new vismodule::Real32*[nvariables];
        face_center_scalars = new vismodule::Real32*[nvariables];
        cell_center_scalars = new vismodule::Real32[nvariables];

        for( int i=0; i<nvariables; i++ )
        {
            pyramidal_scalars[i] = new vismodule::Real32[5];
            for( int j=0; j<5; j++ )
            {
                pyramidal_scalars[i][j] = m_values[i][local_id[j]];
            }

            face_center_scalars[i] = new vismodule::Real32[1];
            fid = 0;
            for( int j=0; j<1; j++, fid+=4 )
            {
                face_center_scalars[i][j] = (
                    pyramidal_scalars[i][ face_id[fid  ] ] + pyramidal_scalars[i][ face_id[fid+1] ] +
                    pyramidal_scalars[i][ face_id[fid+2] ] + pyramidal_scalars[i][ face_id[fid+3] ]  ) * 0.25;
            }

            // for AVS
            cell_center_scalars[i] = ( pyramidal_scalars[i][0] );

            // for VTK
            // cell_center_scalars[i] = ( pyramidal_scalars[i][4] ); 
        }

        vismodule::Vec3 vert[4];
        vismodule::Vec4* tetra_scalars;
        tetra_scalars = new vismodule::Vec4[nvariables];

        for( int face=0; face<1; face++ )
        {
            for( int i=0; i<4; i++ )
            {
                vert[0] = cell_center_vertex;
                vert[1] = face_center_vertex[ face ];
                vert[2] = vertex[ face_id[ face*4 + i ] ];
                vert[3] = vertex[ face_id[ face*4 + ((i+1) % 4) ]];

                for( int j=0; j<nvariables; j++ )
                {
                    tetra_scalars[j][0] = cell_center_scalars[j];
                    tetra_scalars[j][1] = face_center_scalars[j][ face ];
                    tetra_scalars[j][2] = pyramidal_scalars[j][ face_id[ face*4 + i ] ];
                    tetra_scalars[j][3] = pyramidal_scalars[j][ face_id[ face*4 + ((i+1) % 4) ] ];                    
                }

                this->sampling_in_tetrahedra( P, vert, tetra_scalars, nvariables );
                if ( m_mask ) break;
            }
            if ( m_mask ) break;
        }

        for( int i=0; i<nvariables; i++ )
        {
            delete[] pyramidal_scalars[i];
            delete[] face_center_scalars[i];
        }

        delete[] pyramidal_scalars;
        delete[] face_center_scalars;
        delete[] cell_center_scalars;
        delete[] tetra_scalars;

        if ( m_mask ) break;
    }
};

void PlotOverTime::for_prismic_mesh( const vismodule::Vec3 P )
{
    const vismodule::Real32* coords = m_coordinates;
    const vismodule::UInt32* connec = m_connections;
    const size_t ncells = m_ncells;
    const size_t ncoord = m_nnodes;
    const size_t nvariables = m_nvariables;

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
        if( this->is_point_in_boundingbox( MinCoord, MaxCoord, P ) == false )
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

        vismodule::Real32** prismic_scalars;
        vismodule::Real32** face_center_scalars;
        vismodule::Real32*  cell_center_scalars;
        prismic_scalars     = new vismodule::Real32*[nvariables];
        face_center_scalars = new vismodule::Real32*[nvariables];
        cell_center_scalars = new vismodule::Real32[nvariables];

        for( int i=0; i<nvariables; i++ )
        {
            prismic_scalars[i] = new vismodule::Real32[6];
            for( int j=0; j<6; j++ )
            {
                prismic_scalars[i][j] = m_values[i][ local_id[j] ];
            }

            face_center_scalars[i] = new vismodule::Real32[3];
            fid = 0;
            for( int j=0; j<3; j++, fid+=4 )
            {
                face_center_scalars[i][j] = (
                    prismic_scalars[i][ face_id[fid  ] ] + prismic_scalars[i][ face_id[fid+1] ] +
                    prismic_scalars[i][ face_id[fid+2] ] + prismic_scalars[i][ face_id[fid+3] ]  ) * 0.25;
            }

            cell_center_scalars[i] = ((
                prismic_scalars[i][0]+prismic_scalars[i][1]+prismic_scalars[i][2]+
                prismic_scalars[i][3]+prismic_scalars[i][4]+prismic_scalars[i][5] ) / 6.0 ); 
        }

        vismodule::Vec3 vert[4];
        vismodule::Vec4* tetra_scalars;
        tetra_scalars = new vismodule::Vec4[nvariables];

        for( int face=0; face<3; face++ )
        {
            for( int i=0; i<4; i++ )
            {
                vert[0] = cell_center_vertex;
                vert[1] = face_center_vertex[ face ];
                vert[2] = vertex[ face_id[ face*4 + i ] ];
                vert[3] = vertex[ face_id[ face*4 + ((i+1) % 4) ]];

                for ( int j=0; j<nvariables; j++ )
                {
                    tetra_scalars[j][0] = cell_center_scalars[j];
                    tetra_scalars[j][1] = face_center_scalars[j][ face ];
                    tetra_scalars[j][2] = prismic_scalars[j][ face_id[ face*4 + i ] ];
                    tetra_scalars[j][3] = prismic_scalars[j][ face_id[ face*4 + ((i+1) % 4) ] ];
                }

                this->sampling_in_tetrahedra( P, vert, tetra_scalars, nvariables );
                if ( m_mask ) break;
            }
            if ( m_mask ) break;
        }

        // Upper triangle
        if ( !m_mask )
        {
            vert[0] = cell_center_vertex;
            vert[1] = vertex[2];
            vert[2] = vertex[1];
            vert[3] = vertex[0];

            for( int i=0; i<nvariables; i++ )
            {
                tetra_scalars[i][0] = cell_center_scalars[i];
                tetra_scalars[i][1] = prismic_scalars[i][2];
                tetra_scalars[i][2] = prismic_scalars[i][1];
                tetra_scalars[i][3] = prismic_scalars[i][0];
            }

            this->sampling_in_tetrahedra( P, vert, tetra_scalars, nvariables );
        }

        // Lower triangle
        if ( !m_mask )
        {
            vert[0] = cell_center_vertex;
            vert[1] = vertex[3];
            vert[2] = vertex[4];
            vert[3] = vertex[5];

            for( int i=0; i<nvariables; i++ )
            {
                tetra_scalars[i][0] = cell_center_scalars[i];
                tetra_scalars[i][1] = prismic_scalars[i][3];
                tetra_scalars[i][2] = prismic_scalars[i][4];
                tetra_scalars[i][3] = prismic_scalars[i][5];
            }

            this->sampling_in_tetrahedra( P, vert, tetra_scalars, nvariables );
        }

        for( int i=0; i<nvariables; i++ )
        {
            delete[] prismic_scalars[i];
            delete[] face_center_scalars[i];
        }

        delete[] prismic_scalars;
        delete[] face_center_scalars;
        delete[] cell_center_scalars;
        delete[] tetra_scalars;

        if ( m_mask ) break;
    } // end of for cell
};

void PlotOverTime::for_polyhedral_mesh( const vismodule::Vec3 P )
{
    const vismodule::Real32* coords = m_polyhedron->coords.pointer();
    const vismodule::UInt32* connec = m_polyhedron->connections.pointer();
    const vismodule::UInt32* conoff = m_polyhedron->connection_offsets.pointer();
    const vismodule::UInt32* faces  = m_polyhedron->faces.pointer();
    const vismodule::UInt32* facoff = m_polyhedron->face_offsets.pointer();
    const vismodule::Real32* scalar = m_polyhedron->scalars.pointer();
    const size_t ncells     = m_polyhedron->ncells;
    const size_t nvariables = 1;

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
        if( this->is_point_in_boundingbox( MinCoord, MaxCoord, P ) == false )
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
            vismodule::Vec4* s;
            s = new vismodule::Vec4[nvariables];

            for( int i=0; i<nfacevert; i++ )
            {
                const size_t vid2 = *(pface + i);
                const size_t vid3 = *(pface + (i+1)%nfacevert);
                vert[0] = cell_center_vertex;
                vert[1] = face_center_vertex;
                vert[2] = vismodule::Vec3( coords[vid2*3], coords[vid2*3+1], coords[vid2*3+2] );
                vert[3] = vismodule::Vec3( coords[vid3*3], coords[vid3*3+1], coords[vid3*3+2] );

                for( int j=0; j<nvariables; j++ )
                {
                    s[j][0] = cell_center_scalar;
                    s[j][1] = face_center_scalar;
                    s[j][2] = scalar[ vid2 ];
                    s[j][3] = scalar[ vid3 ];
                }

                this->sampling_in_tetrahedra( P, vert, s, nvariables );
                if ( m_mask ) break;
            }

            delete[] s;
            if ( m_mask ) break;
        } // end of for each face
        if ( m_mask ) break;
    } // end of for each cell
};

void PlotOverTime::sampling_in_tetrahedra(
    const vismodule::Vec3 P,
    const vismodule::Vec3* vertices,
    const vismodule::Vec4* scalars,
    const int nvariables
)
{
    // Tetrahedra vertices and scalars
    const vismodule::Vec3 X0 = vertices[0];
    const vismodule::Vec3 X1 = vertices[1]; 
    const vismodule::Vec3 X2 = vertices[2];
    const vismodule::Vec3 X3 = vertices[3];

    // Bounding Box
    vismodule::Vec3 MinCoord(
        vismodule::Math::Min<float>( X0.x(), X1.x(), X2.x(), X3.x() ),
        vismodule::Math::Min<float>( X0.y(), X1.y(), X2.y(), X3.y() ),
        vismodule::Math::Min<float>( X0.z(), X1.z(), X2.z(), X3.z() )
    );

    vismodule::Vec3 MaxCoord(
        vismodule::Math::Max<float>( X0.x(), X1.x(), X2.x(), X3.x() ),
        vismodule::Math::Max<float>( X0.y(), X1.y(), X2.y(), X3.y() ),
        vismodule::Math::Max<float>( X0.z(), X1.z(), X2.z(), X3.z() )
    );

    if( this->is_point_in_boundingbox( MinCoord, MaxCoord, P ) == false )
    {
        // std::cout << __LINE__ << ":" << "Point P is not in tet's bounding box." << std::endl;
        return;
    }

    // Barycentric coordinates of P := ( a0, b0, c0, d0 ).
    const vismodule::Vec4 bc_P = this->barycentric_coordinates( X0, X1, X2, X3, P );

    // In/out test: Use an epsilon to allow for small numerical errors.
    // If any of the barycentric coordinates are negative,
    // the point is not in tet's bounding box.
    const float eps = -1e-6f;
    if ( bc_P.x() < eps || bc_P.y() < eps || bc_P.z() < eps || bc_P.w() < eps )
    {
        // std::cout << __LINE__ << ":" << "Point P is not in tet's bounding box." << std::endl;
        return;
    }

    m_mask = true;

    for ( int i = 0; i < nvariables; i++ )
    {
        const vismodule::Vec4 S( scalars[i] );
        m_values_on_time[i] = bc_P.dot(S);
    }

    return;
}

const bool PlotOverTime::is_point_in_boundingbox(
    const vismodule::Vec3 MinCoord,
    const vismodule::Vec3 MaxCoord,
    const vismodule::Vec3 P
)
{
    // 各軸 (X, Y, Z) についてチェック
    for( int i=0; i<3; i++ )
    {
        // 点の座標が最小値より小さい、または最大値より大きい場合は外側
        if( P[i] < MinCoord[i] || MaxCoord[i] < P[i] )
        {
            return false;
        }
    }

    // すべての軸で範囲内であれば内側
    return true;
}

// Barycentric coordinates := (a, b, c, d)
// Here, P = a X0 + b X1 + c X2 + d X3
// a+b+c+d=1 ( a=V0/V, b=V1/V,...  makes this condition true.)
// if a, b, c, d >= 0 , P is included in tetrahedra
// this method is free for the direction of tetrahedra.
const vismodule::Vec4 PlotOverTime::barycentric_coordinates(
    const vismodule::Vec3 X0,
    const vismodule::Vec3 X1,
    const vismodule::Vec3 X2,
    const vismodule::Vec3 X3,
    const vismodule::Vec3 P
)
{
    const float V  = tetrahedra_signed_volume_X6( X0, X1, X2, X3 );
    const float V0 = tetrahedra_signed_volume_X6(  P, X1, X2, X3 );
    const float V1 = tetrahedra_signed_volume_X6( X0,  P, X2, X3 );
    const float V2 = tetrahedra_signed_volume_X6( X0, X1,  P, X3 );
    const float V3 = tetrahedra_signed_volume_X6( X0, X1, X2,  P );

    return vismodule::Vec4( V0/V, V1/V, V2/V, V3/V );
}

// Tetrahedral volume is M.determinant/6.
// Because barycentric coordinated is ratio of tetrahedral volume, 1/6 is negligible.
const float PlotOverTime::tetrahedra_signed_volume_X6(
    const vismodule::Vec3 X0,
    const vismodule::Vec3 X1,
    const vismodule::Vec3 X2,
    const vismodule::Vec3 X3
)
{
    const vismodule::Vec3 X0_3 = X0 - X3;
    const vismodule::Vec3 X1_3 = X1 - X3;
    const vismodule::Vec3 X2_3 = X2 - X3;
    const vismodule::Mat3 M(
        X0_3.x(), X1_3.x(), X2_3.x(),
        X0_3.y(), X1_3.y(), X2_3.y(),
        X0_3.z(), X1_3.z(), X2_3.z()
    );

    return M.determinant();
}
