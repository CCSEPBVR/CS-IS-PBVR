// 使用箇所は全てomp parallel内でそれを前gに作成している
inline
void calculateMinmax_struct ( const int nx, const int ny, const int nz,
        const vismodule::Vector3f min_vec, vismodule::Vector3f cell_length,
        const int nvariables, int thid, int tf_number,
       std::vector< std::vector< vismodule::TrilinearInterpolator* > >& interp, TransferFunctionSynthesizer** th_tfs, 
       float** o_scalars, float** c_scalars,
       vismodule::ValueArray<float>& th_O_min, vismodule::ValueArray<float>& th_O_max, vismodule::ValueArray<float>& th_C_min, vismodule::ValueArray<float>& th_C_max,  
       vismodule::ValueArray<float>& O_min, vismodule::ValueArray<float>& O_max, vismodule::ValueArray<float>& C_min, vismodule::ValueArray<float>& C_max )
{

            // Marge x-y-z loop
            const int nvertices = nx * ny * nz;
            // "+ 1" means remained loop
            const int outer_loop = (nvertices % SIMDW == 0) ?
                nvertices / SIMDW : nvertices / SIMDW + 1;
            const int nxy = nx * ny;

            #pragma omp for
            for( int J=0; J<outer_loop; J++ )
            {
                int ncells = ((J+1) * SIMDW > nvertices) ? nvertices - J*SIMDW  : SIMDW ;
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];
                #pragma ivdep
                for( int I=0; I<SIMDW; I++ )
                {
                    const int vertex_id = I + J * SIMDW;
                    // vertex_id = i + j * nx + k * nx * ny
                    const int k =  vertex_id / nxy;
                    const int j = (vertex_id - k * nxy) / nx;
                    const int i =  vertex_id - k * nxy - j * nx;

                    const float x_l = (float)i;
                    const float y_l = (float)j;
                    const float z_l = (float)k;
                    const float x_g = (x_l * cell_length.x())+min_vec.x();
                    const float y_g = (y_l * cell_length.y())+min_vec.y();
                    const float z_g = (z_l * cell_length.z())+min_vec.z();

                    X_l[I] = x_l;
                    Y_l[I] = y_l;
                    Z_l[I] = z_l;
                    X_g[I] = x_g;
                    Y_g[I] = y_g;
                    Z_g[I] = z_g;
                }
                th_tfs[thid]->SynthesizedOpacityScalars(
                    interp[thid], X_l, Y_l, Z_l, X_g, Y_g, Z_g, o_scalars );
                th_tfs[thid]->SynthesizedColorScalars(
                    interp[thid], X_l, Y_l, Z_l, X_g, Y_g, Z_g, c_scalars );

                for( int i = 0; i < tf_number; i++ )
                {
                    for( int I = 0; I < ncells; I++ )
                    {
                        th_O_min[i] = th_O_min[i] < o_scalars[i][I] ? th_O_min[i] : o_scalars[i][I];
                        th_O_max[i] = th_O_max[i] > o_scalars[i][I] ? th_O_max[i] : o_scalars[i][I];
                        th_C_min[i] = th_C_min[i] < c_scalars[i][I] ? th_C_min[i] : c_scalars[i][I];
                        th_C_max[i] = th_C_max[i] > c_scalars[i][I] ? th_C_max[i] : c_scalars[i][I];
                    }
                }

            }

#pragma omp critical
            {
                for( int i = 0; i < tf_number; i++ )
                {
                    O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                    O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                    C_min[i] = C_min[i] < th_C_min[i] ? C_min[i] : th_C_min[i];
                    C_max[i] = C_max[i] > th_C_max[i] ? C_max[i] : th_C_max[i];
                }
            } // end of omp critical
}

inline
void calculateHistogram_struct( const int nx, const int ny, const int nz,
        const vismodule::Vector3f min_vec, vismodule::Vector3f cell_length,
        const int nvariables, int thid, int tf_number,
       std::vector< std::vector< vismodule::TrilinearInterpolator* > >& interp, TransferFunctionSynthesizer** th_tfs, 
       float** o_scalars, float** c_scalars,
        vismodule::ValueArray<float>& o_min, vismodule::ValueArray<float>& o_max, vismodule::ValueArray<float>& c_min, vismodule::ValueArray<float>& c_max, 
        vismodule::ValueArray<int>& th_o_histogram, vismodule::ValueArray<int>& th_c_histogram )
{
    int nbins = 256;
    // Marge x-y-z loop
    const int nvertices = nx * ny * nz;
    // "+ 1" means remained loop
    const int outer_loop = (nvertices % SIMDW == 0) ?
        nvertices / SIMDW : nvertices / SIMDW + 1;
    const int nxy = nx * ny;

            #pragma omp for
            for( int J=0; J<outer_loop; J++ )
            {
                int ncells = ((J+1) * SIMDW > nvertices) ? nvertices - J*SIMDW  : SIMDW ;
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];
                #pragma ivdep
                for( int I=0; I<SIMDW; I++ )
                {
                    const int vertex_id = I + J * SIMDW;
                    // vertex_id = i + j * nx + k * nx * ny
                    const int k =  vertex_id / nxy;
                    const int j = (vertex_id - k * nxy) / nx;
                    const int i =  vertex_id - k * nxy - j * nx;

                    const float x_l = (float)i;
                    const float y_l = (float)j;
                    const float z_l = (float)k;
                    const float x_g = (x_l * cell_length.x())+min_vec.x();
                    const float y_g = (y_l * cell_length.y())+min_vec.y();
                    const float z_g = (z_l * cell_length.z())+min_vec.z();

                    X_l[I] = x_l;
                    Y_l[I] = y_l;
                    Z_l[I] = z_l;
                    X_g[I] = x_g;
                    Y_g[I] = y_g;
                    Z_g[I] = z_g;
                }
                th_tfs[thid]->SynthesizedOpacityScalars(
                    interp[thid], X_l, Y_l, Z_l, X_g, Y_g, Z_g, o_scalars );
                th_tfs[thid]->SynthesizedColorScalars(
                    interp[thid], X_l, Y_l, Z_l, X_g, Y_g, Z_g, c_scalars );
                for( int i = 0; i < tf_number; i++ )
                {
                    for( int I = 0; I < ncells; I++ )
                    {
                        //ｿｿｿｿｿｿｿｿｿｿｿ
                        float h = (o_scalars[i][I] - o_min[i])/( o_max[i] - o_min[i] )*nbins;
                        int H = (int)h;
                        if( 0 <= H && H <= nbins )
                        {
                            if( H == nbins ) H--;
                            th_o_histogram[ H + nbins*i]++;
                        }

                        //ｿｿｿｿｿｿｿｿ
                        h = (c_scalars[i][I] - c_min[i])/( c_max[i] - c_min[i] )*nbins;
                        H = (int)h;
                        if( 0 <= H && H <= nbins )
                        {
                            if( H == nbins ) H--;
                            th_c_histogram[ H + nbins*i]++;
                        }

                    }
                }

            }
}

inline
void calculateMinmax_unstruct(vismodule::UInt32* cell_index, const int ncells, vismodule::Vector3f* local_center_array, vismodule::Vector3f* global_center_array,
        const int nvariables, int thid, int tf_number,
       std::vector< std::vector< vismodule::CellBase<Type>* > >& interp, TransferFunctionSynthesizer** th_tfs, 
       std::vector<float>* o_scalars_array, std::vector<float>* c_scalars_array,
       vismodule::ValueArray<float>& th_O_min, vismodule::ValueArray<float>& th_O_max, vismodule::ValueArray<float>& th_C_min, vismodule::ValueArray<float>& th_C_max,  
       vismodule::ValueArray<float>& O_min, vismodule::ValueArray<float>& O_max, vismodule::ValueArray<float>& C_min, vismodule::ValueArray<float>& C_max )
{
//    const int SIMDW = 128;
#pragma omp for schedule( dynamic ) nowait  
        for( int cell_base = 0; cell_base < ncells; cell_base += SIMDW )
        {
            //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
            int remain = ( ncells - cell_base > SIMDW )? SIMDW: ncells - cell_base;

            /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (vismodule::UInt32)(cell_base + cell_BLK);
                //local_center_array[cell_BLK] = vismodule::Vector3f ( 0.5, 0.5, 0.5 );
                local_center_array[cell_BLK] = vismodule::Vector3f ( 0, 0, 0 );
            }

            //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
            for(int i = 0; i < nvariables; i++)
            {
                interp[thid][i]->bindCellArray(remain, cell_index);
            }

            interp[thid][0]->setLocalPointArray( remain, local_center_array );
            interp[thid][0]->transformLocalToGlobalArray( remain,
                    local_center_array,
                    global_center_array );

                th_tfs[thid]->SynthesizedOpacityScalarsArray( interp[thid],
                        remain,
                        local_center_array,
                        global_center_array,
                        o_scalars_array );

                th_tfs[thid]->SynthesizedColorScalarsArray( interp[thid],
                        remain,
                        local_center_array,
                        global_center_array,
                        c_scalars_array );

                for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ ) 
                {
                    for( int i = 0; i < tf_number; i++ )
                    {
                        // 20190128 ｿｿ
                        th_O_min[i] = th_O_min[i] < o_scalars_array[cell_BLK][i] ? th_O_min[i] : o_scalars_array[cell_BLK][i];
                        th_O_max[i] = th_O_max[i] > o_scalars_array[cell_BLK][i] ? th_O_max[i] : o_scalars_array[cell_BLK][i];
                        th_C_min[i] = th_C_min[i] < c_scalars_array[cell_BLK][i] ? th_C_min[i] : c_scalars_array[cell_BLK][i];
                        th_C_max[i] = th_C_max[i] > c_scalars_array[cell_BLK][i] ? th_C_max[i] : c_scalars_array[cell_BLK][i];
                    }
                }
        }// end of for cell


#pragma omp critical
        {
            for( int i = 0; i < tf_number; i++ )
            {
                O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                C_min[i] = C_min[i] < th_C_min[i] ? C_min[i] : th_C_min[i];
                C_max[i] = C_max[i] > th_C_max[i] ? C_max[i] : th_C_max[i];
            }
        }

}

inline
void calculateHistogram_unstruct(
        vismodule::UInt32* cell_index, int ncells, vismodule::Vector3f* local_center_array, vismodule::Vector3f* global_center_array,
        const int nvariables, int thid, int tf_number,
        std::vector< std::vector< vismodule::CellBase<Type>* > >& interp, TransferFunctionSynthesizer** th_tfs, 
        std::vector<float>* o_scalars_array, std::vector<float>* c_scalars_array,
        vismodule::ValueArray<float>& o_min, vismodule::ValueArray<float>& o_max, vismodule::ValueArray<float>& c_min, vismodule::ValueArray<float>& c_max, 
        vismodule::ValueArray<int>& th_o_histogram, vismodule::ValueArray<int>& th_c_histogram )
{
    int nbins = 256;
#pragma omp for schedule( dynamic ) nowait  
        for( int cell_base = 0; cell_base < ncells; cell_base += SIMDW )
        {
            int remain = ( ncells - cell_base > SIMDW )? SIMDW: ncells - cell_base;

            /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (vismodule::UInt32)(cell_base + cell_BLK);
                local_center_array[cell_BLK] = vismodule::Vector3f ( 0, 0, 0 );
            }

            for(int i = 0; i < nvariables; i++)
            {
                interp[thid][i]->bindCellArray(remain, cell_index);
            }

            interp[thid][0]->setLocalPointArray( remain, local_center_array );
            interp[thid][0]->transformLocalToGlobalArray( remain,
                    local_center_array,
                    global_center_array );


                th_tfs[thid]->SynthesizedOpacityScalarsArray( interp[thid],
                        remain,
                        local_center_array,
                        global_center_array,
                        o_scalars_array );

                th_tfs[thid]->SynthesizedColorScalarsArray( interp[thid],
                        remain,
                        local_center_array,
                        global_center_array,
                        c_scalars_array );

                for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ ) 
                {
                    for( int i = 0; i < tf_number; i++ )
                    {
                        float h = (o_scalars_array[cell_BLK][i] - o_min[i])/( o_max[i] - o_min[i] )*nbins;
                        int H = (int)h;
                        if( 0 <= H && H <= nbins )
                        {
                            if( H == nbins ) H--;
                            th_o_histogram[ H + nbins*i]++;
                        }

                        h = (c_scalars_array[cell_BLK][i] - c_min[i])/( c_max[i] - c_min[i] )*nbins;
                        H = (int)h;
                        if( 0 <= H && H <= nbins )
                        {
                            if( H == nbins ) H--;
                            th_c_histogram[ H + nbins*i]++;
                        }
                    }
                }
            /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
        }// end of for cell


}
