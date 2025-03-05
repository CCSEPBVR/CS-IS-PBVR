#ifndef PLOT_OVER_LINE 
#define PLOT_OVER_LINE
#include <kvs/Matrix>
#include <kvs/Vector>
#include <kvs/Math>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/StructuredVolumeObject>
#include "StructuredVolumeObject.h"
#ifndef CPU_VER
#include <mpi.h>
#endif


#include <iomanip>
#include "PlotOverLineProperty.h"
namespace kvs
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
    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::Real32> scalars;
    kvs::ValueArray<kvs::UInt32> connections;
    kvs::ValueArray<kvs::UInt32> connection_offsets;
    kvs::ValueArray<kvs::UInt32> faces;
    kvs::ValueArray<kvs::UInt32> face_offsets;
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
    kvs::ValueArray<float> m_values_on_line;
    kvs::ValueArray<float> m_x_axis;
    kvs::ValueArray<bool>  m_mask;
    
    // 全セルタイプのデータ
    kvs::ValueArray<float> m_allcell_values_on_line;
    //kvs::ValueArray<float> m_allcell_x_axis;
    kvs::ValueArray<bool>  m_allcell_mask;
    
//    const kvs::StructuredVolumeObject* m_structured_volume;
    const pbvr::StructuredVolumeObject* m_structured_volume;
    const kvs::UnstructuredVolumeObject* m_volume;
    const POL::Polyhedron* m_polyhedron;

    // 生成判定フラグ
    bool m_plot_flag;
    // ファイルパス(デーモン→サーバー)
    std::string m_POLParamPath;
    // ファイルパス(サーバー→デーモン)
    std::string m_POLFilePath;
    //始点
    kvs::Vec3 m_start_point; 
    //終点
    kvs::Vec3 m_end_point; 
    //解像度
    int m_resolution;
    //指定変数
    int m_plot_variable ;


public:
    PlotOverLine( void );

    //PlotOverLine( const kvs::StructuredVolumeObject* volume,
    PlotOverLine( const pbvr::StructuredVolumeObject* volume,
                            const size_t resolution,
                            const kvs::Vec3 P0, const kvs::Vec3 P1 );

    PlotOverLine( const kvs::UnstructuredVolumeObject* volume,
                  const size_t resolution,
                  const kvs::Vec3 P0, const kvs::Vec3 P1 );

    // CS用
    PlotOverLine( const kvs::UnstructuredVolumeObject* volume,
                  const size_t resolution,
                  const kvs::Vec3 P0, const kvs::Vec3 P1 , const int plot_variable);

    // CS用
    PlotOverLine( const pbvr::StructuredVolumeObject* volume,
                  const size_t resolution,
                  const kvs::Vec3 P0, const kvs::Vec3 P1 , const int plot_variable);



    PlotOverLine( const POL::Polyhedron* volume,
                  const size_t resolution,
                  const kvs::Vec3 P0, const kvs::Vec3 P1 );

    ~PlotOverLine();

//    void setVolume( const kvs::StructuredVolumeObject* volume );
    void setVolume( const pbvr::StructuredVolumeObject* volume );
    void setVolume( const kvs::UnstructuredVolumeObject* volume );
    void setVolume( const POL::Polyhedron* volume );

    void setResolution( const size_t resolution );
    void extractPlotLineStructured( const kvs::Vec3 P0, const kvs::Vec3 P1 );
    void extractPlotLine( const kvs::UnstructuredVolumeObject* volume );
    void extractPlotLine( const kvs::Vec3 P0, const kvs::Vec3 P1 );
    void extractPlotLinePoly( const kvs::Vec3 P0, const kvs::Vec3 P1 );

    bool SetPOLParameter( const int time_step);
    void CellTypeReduceing();
    void OutputLine(const int time_step);
    

    int resolution() {return m_resolution;}
    bool plot_flag(){return m_plot_flag;}
    kvs::ValueArray<float> values(){ return m_values_on_line;}
    kvs::ValueArray<float> xAxis(){ return m_x_axis;}
    kvs::ValueArray<bool>  mask(){ return m_mask; }
    kvs::Vec3 startPoint(){return m_start_point; }
    kvs::Vec3 endPoint(){return m_end_point; }

    void setValuesOnLine( const kvs::ValueArray<float>& values_on_line ) { m_values_on_line = values_on_line; }
    void setXAxis( const kvs::ValueArray<float>& x_axis ){ m_x_axis = x_axis; }
    void setMask( const kvs::ValueArray<bool>& mask ){ m_mask = mask; }
protected:
    void calculate_x_axis( const kvs::Vec3 P0, const kvs::Vec3 P1 );

    void for_structured_mesh( const kvs::Vec3 P0, const kvs::Vec3 P1 );
    void for_tetrahedral_mesh( const kvs::Vec3 P0, const kvs::Vec3 P1 );
    void for_hexahedral_mesh( const kvs::Vec3 P0, const kvs::Vec3 P1 );
    void for_pyramidal_mesh( const kvs::Vec3 P0, const kvs::Vec3 P1 );
    void for_prismic_mesh( const kvs::Vec3 P0, const kvs::Vec3 P1 );
    void for_polyhedral_mesh( const kvs::Vec3 P0, const kvs::Vec3 P1 );
 
    void sampling_in_tetrahedra( const kvs::Vec3 P0, const kvs::Vec3 P1,
                                 const kvs::Vec3* vertices,
                                 const kvs::Vec4 scalars );

    const bool intersection_of_boundingbox( const kvs::Vec3 MinCoord,
                                            const kvs::Vec3 MaxCoord,
                                            const kvs::Vec3 P0,
                                            const kvs::Vec3 P1 );

    const float tetrahedra_signed_volume_X6(
        const kvs::Vec3 X0, const kvs::Vec3 X1, const kvs::Vec3 X2, const kvs::Vec3 X3 );

    const kvs::Vec4 barycentric_coordinates(
        const kvs::Vec3 X0, const kvs::Vec3 X1, const kvs::Vec3 X2, const kvs::Vec3 X3,
        const kvs::Vec3 P );

    POL::Range t_range_in_tet( const kvs::Vec4 bc_P0, const kvs::Vec4 bc_P1 );
};
#endif
