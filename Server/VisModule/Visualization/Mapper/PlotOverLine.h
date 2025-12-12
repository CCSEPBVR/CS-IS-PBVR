#ifndef PLOT_OVER_LINE 
#define PLOT_OVER_LINE
#include <vismodule/Matrix>
#include <vismodule/Vector>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/PlotOverLineProperty>

namespace vismodule
{
//    using Vec3 = Vector3<float>;
//    using Vec4 = Vector4<float>;
//    using Mat3 =Matrix33<float>;
    using Vec3 = Vector3<double>;
    using Vec4 = Vector4<double>;
    using Mat3 =Matrix33<double>;
}
namespace POL
{

class Polyhedron
{
public:
    size_t nvertices;
    size_t ncells;
    vismodule::ValueArray<vismodule::Real32> coords;
    vismodule::ValueArray<vismodule::Real32> scalars;
    vismodule::ValueArray<vismodule::UInt32> connections;
    vismodule::ValueArray<vismodule::UInt32> connection_offsets;
    vismodule::ValueArray<vismodule::UInt32> faces;
    vismodule::ValueArray<vismodule::UInt32> face_offsets;
};

class Range
{
private:
    float m_lower;
    float m_upper;
    bool  m_non_empty;

public:
    Range():m_lower(0.0), m_upper(0.0), m_non_empty(true){}

    Range( float lower, float upper )
        {
            m_lower = lower;
            m_upper = upper;

            if( lower > upper )
            {
                //std::cout << "Lower cannot be greater than Upper" << std::endl;
                m_non_empty = false;
            }
            else m_non_empty = true;
        }

    void set( float lower, float upper )
        {
            m_lower = lower;
            m_upper = upper;

            if( lower > upper )
            {
                //std::cout << "Lower cannot be greater than Upper" << std::endl;
                m_non_empty = false;
            }
            else m_non_empty = true;
        }

    const float lower( ) const { return m_lower; }
    const float upper( ) const { return m_upper; }
    const bool  non_empty() const { return m_non_empty; }

    // AND ( intersect )
    const Range intersect( const Range& other ) const
        {
            const float new_lower = (std::max)( m_lower, other.lower() );
            const float new_upper = (std::min)( m_upper, other.upper() );

            if( new_lower <= new_upper )
            {
                return Range( new_lower, new_upper );
            }
            else
            {
                //std::cout << "separated regions" << std::endl;
                return Range( 1, -1 );
            }
        }

    // OR ( merge )
    const Range merge( const Range& other ) const
        {
            if( other.lower() <= m_upper && m_lower <= other.upper() )
            {
                const float new_lower = (std::min)( m_lower, other.lower() ); // ( (std::min), (std::max)) はwindowsのビルド対応
                const float new_upper = (std::max)( m_upper, other.upper() );
                return Range( new_lower, new_upper );
            }
            else
            {
                //std::cout << "separated regions" << std::endl;
                return Range( 2, -2 );
            }
        }

    Range& operator=(const Range& rhs )
        {
            m_lower = rhs.m_lower;
            m_upper = rhs.m_upper;
            m_non_empty = rhs.m_non_empty;
            return *this;
        }
    
    void print() const { std::cout << "[" << m_lower << ", " << m_upper << "]" << std::endl; }
};
} // end of namespace POL

class PlotOverLine
{
private:
    // セルタイプ別のデータ
    vismodule::ValueArray<float> m_values_on_line;
    vismodule::ValueArray<float> m_x_axis;
    vismodule::ValueArray<bool>  m_mask;
    
    // 全セルタイプのデータ
    vismodule::ValueArray<float> m_allcell_values_on_line;
    //vismodule::ValueArray<float> m_allcell_x_axis;
    vismodule::ValueArray<bool>  m_allcell_mask;
    
    const vismodule::StructuredVolumeObject* m_structured_volume;
    const vismodule::UnstructuredVolumeObject* m_volume;
    const POL::Polyhedron* m_polyhedron;

    // 変数配列
    float **m_values;
    int m_nvariables;
    float* m_coordinates;
    int m_nnodes;
    unsigned int* m_connections;
    int m_ncells;
    vismodule::VolumeObjectBase::CellType m_cellType; 

    size_t m_resolution_x;
    size_t m_resolution_y;
    size_t m_resolution_z;

    domain_parameters_struct m_dom;    

    // 生成判定フラグ
    bool m_plot_flag;
    // ファイルパス(デーモン→サーバー)
    std::string m_POLParamPath;
    // ファイルパス(サーバー→デーモン)
    std::string m_POLFilePath;
    //始点
    vismodule::Vec3 m_start_point; 
    //終点
    vismodule::Vec3 m_end_point; 
    //解像度
    int m_sampling_size;
    //指定変数
    int m_plot_variable ;

    //cell_length
    float m_cell_length;
    
    // offset
    vismodule::Vec3 m_offset;

public:
    PlotOverLine( void );

    PlotOverLine( const vismodule::StructuredVolumeObject* volume,
//    PlotOverLine( const pbvr::StructuredVolumeObject* volume,
                            const size_t sampling_size,
                            const vismodule::Vec3 P0, const vismodule::Vec3 P1 );
                        
    PlotOverLine( const vismodule::UnstructuredVolumeObject* volume,
                  const size_t sampling_size,
                  const vismodule::Vec3 P0, const vismodule::Vec3 P1 );

    //変数配列用コンストラクター
    PlotOverLine( Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& celltype, 
        const size_t sampling_size,
        const vismodule::Vec3 P0, const vismodule::Vec3 P1, const int plot_variable );


    PlotOverLine( domain_parameters_struct dom, float** values, int nvariables, 
        const size_t sampling_size,
        const vismodule::Vec3 P0, const vismodule::Vec3 P1, const int plot_variable );

    // unstruct
    PlotOverLine(
        Type** values,
        int nvariables,
        float* coordinates,
        int ncoords,
        unsigned int* connections,
        int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype, 
        const PlotOverLineProperty& pol_property
    );

    //struct 
    PlotOverLine(
        domain_parameters_struct dom,
        float** values,
        int nvariables, 
        const PlotOverLineProperty& pol_property
    );


    // CS用
    PlotOverLine( const vismodule::UnstructuredVolumeObject* volume,
                  const size_t sampling_size,
                  const vismodule::Vec3 P0, const vismodule::Vec3 P1 , const int plot_variable);

    // CS用
    PlotOverLine( const vismodule::StructuredVolumeObject* volume,
                  const size_t sampling_size,
                  const vismodule::Vec3 P0, const vismodule::Vec3 P1 , const int plot_variable);


    PlotOverLine( const POL::Polyhedron* volume,
                  const size_t sampling_size,
                  const vismodule::Vec3 P0, const vismodule::Vec3 P1 );

    ~PlotOverLine();

    void setVolume( const vismodule::StructuredVolumeObject* volume );
    void setVolume( const vismodule::UnstructuredVolumeObject* volume );
    void extractPlotLine( float** values, int nvariables,
            float* coordinates, int ncoords,
            unsigned int* connections, int ncells,
            const  vismodule::VolumeObjectBase::CellType& celltype );

    void setVolume( const POL::Polyhedron* volume );

    void setSamplingSize( const size_t sampling_size );
    void extractPlotLineStructured( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );
    void extractPlotLine( const vismodule::StructuredVolumeObject* volume );
    void extractPlotLine( const vismodule::UnstructuredVolumeObject* volume );
//    void extractPlotLine( float** values, int nvariables,
//            float* coordinates, int ncoords,
//            unsigned int* connections, int ncells,
//            const  vismodule::VolumeObjectBase::CellType& celltype );
    void extractPlotLine( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );
    void extractPlotLinePoly( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );

    bool SetPOLParameter( const int time_step);
    void CellTypeReduceing();
    void OutputLine(const int time_step);
    

    int sampling_size() {return m_sampling_size;}
    bool plot_flag(){return m_plot_flag;}
    vismodule::ValueArray<float> values(){ return m_values_on_line;}
    vismodule::ValueArray<float> xAxis(){ return m_x_axis;}
    vismodule::ValueArray<bool>  mask(){ return m_mask; }
    vismodule::Vec3 startPoint(){return m_start_point; }
    vismodule::Vec3 endPoint(){return m_end_point; }

    void setValuesOnLine( const vismodule::ValueArray<float>& values_on_line ) { m_values_on_line = values_on_line; }
    void setXAxis( const vismodule::ValueArray<float>& x_axis ){ m_x_axis = x_axis; }
    void setMask( const vismodule::ValueArray<bool>& mask ){ m_mask = mask; }
    void SetCellLength(const float cell_length ) { m_cell_length = cell_length ;}
    void SetOffset(const vismodule::Vector3f offset ) { m_offset.x() = offset.x(); m_offset.y() = offset.y(); m_offset.z() = offset.z();} 
protected:
    void calculate_x_axis( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );

    void for_structured_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );
    void for_tetrahedral_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );
    void for_hexahedral_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );
    void for_pyramidal_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );
    void for_prismic_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );
    void for_polyhedral_mesh( const vismodule::Vec3 P0, const vismodule::Vec3 P1 );
 
    void sampling_in_tetrahedra( const vismodule::Vec3 P0, const vismodule::Vec3 P1,
                                 const vismodule::Vec3* vertices,
                                 const vismodule::Vec4 scalars );

    const bool intersection_of_boundingbox( const vismodule::Vec3 MinCoord,
                                            const vismodule::Vec3 MaxCoord,
                                            const vismodule::Vec3 P0,
                                            const vismodule::Vec3 P1 );

    const float tetrahedra_signed_volume_X6(
        const vismodule::Vec3 X0, const vismodule::Vec3 X1, const vismodule::Vec3 X2, const vismodule::Vec3 X3 );

    const vismodule::Vec4 barycentric_coordinates(
        const vismodule::Vec3 X0, const vismodule::Vec3 X1, const vismodule::Vec3 X2, const vismodule::Vec3 X3,
        const vismodule::Vec3 P );

    POL::Range t_range_in_tet( const vismodule::Vec4 bc_P0, const vismodule::Vec4 bc_P1 );
};
#endif
