/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__AVS_FLD_H_INCLUDE
#define CVT__AVS_FLD_H_INCLUDE
#include <algorithm>
#include <fstream>
#include <memory>
#include <string>

#include "kvs/FileFormatBase"
#include "kvs/Message"
#include <vtkRectilinearGrid.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>
#include <vtkUniformGrid.h>

#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "Filesystem.h"

namespace cvt
{
namespace detail
{

class AvsFieldHeader
{
public:
    AvsFieldHeader():
        nstep( 1 ),
        ndim( 3 ),
        dim1( 1 ),
        dim2( 1 ),
        dim3( 1 ),
        nspace( 3 ),
        veclen( 0 ),
        data( "float" ),
        field( "union" ),
        has_data_section( false )
    {
    }

public:
    template <typename Stream>
    void print( Stream& stream, int indent_count = 0 )
    {
        std::string indent( indent_count, ' ' );

        stream << indent << "nstep: " << nstep << std::endl;
        stream << indent << "ndim: " << ndim << std::endl;
        stream << indent << "dim1: " << dim1 << std::endl;
        stream << indent << "dim2: " << dim2 << std::endl;
        stream << indent << "dim3: " << dim3 << std::endl;
        stream << indent << "nspace: " << nspace << std::endl;
        stream << indent << "veclen: " << veclen << std::endl;
        stream << indent << "data: " << data << std::endl;
        stream << indent << "field: " << data << std::endl;
        stream << indent << "has data section: " << std::boolalpha << has_data_section << std::endl;
    }

public:
    int nstep;
    int ndim;
    int dim1;
    int dim2;
    int dim3;
    int nspace;
    int veclen;
    std::string data;
    std::string field;
    bool has_data_section;
};

class AvsFieldDataMeta
{
public:
    AvsFieldDataMeta():
        type( "coord" ),
        n( 1 ),
        file( nullptr ),
        filetype( "binary" ),
        skip( 0 ),
        offset( 0 ),
        stride( 1 ),
        close( 0 ),
        value( nullptr )
    {
    }

public:
    std::string type;
    int n;
    std::shared_ptr<std::string> file;
    std::string filetype;
    int skip;
    int offset;
    int stride;
    int close;
    std::shared_ptr<std::string> value;

public:
    template <typename Stream>
    void print( Stream& stream, int indent_count = 0 )
    {
        std::string i( indent_count, ' ' );
        std::string indent( indent_count + 2, ' ' );

        stream << i << "n: " << n << std::endl;
        stream << indent << "type: " << type << std::endl;
        if ( value )
        {
            stream << indent << "value: " << *file << std::endl;
        }
        else
        {
            std::string indent( indent_count + 2, ' ' );
            if ( file )
            {
                stream << indent << "file: " << *file << std::endl;
            }
            else
            {
                stream << indent << "file: nullptr" << std::endl;
            }
            stream << indent << "filetype: " << filetype << std::endl;
            stream << indent << "skip: " << skip << std::endl;
            stream << indent << "offset: " << offset << std::endl;
            stream << indent << "stride: " << stride << std::endl;
            stream << indent << "close: " << close << std::endl;
        }
    }
};

class AvsFieldTimeDependentField
{
public:
    AvsFieldTimeDependentField(): coords( 3, nullptr ), variables() {}

public:
    AvsFieldTimeDependentField& operator=( const AvsFieldTimeDependentField& other )
    {
        coords = other.coords;
        variables = other.variables;

        return *this;
    }

public:
    void SetVeclen( int veclen )
    {
        variables.resize( veclen );
        std::fill( variables.begin(), variables.end(), nullptr );
    }

public:
    std::shared_ptr<cvt::detail::AvsFieldDataMeta> time;
    std::vector<std::shared_ptr<cvt::detail::AvsFieldDataMeta>> coords;
    std::vector<std::shared_ptr<cvt::detail::AvsFieldDataMeta>> variables;

public:
    template <typename Stream>
    void print( Stream& stream, int indent_count = 0 )
    {
        std::string indent( indent_count, ' ' );
        std::string sub_indent( indent_count + 2, ' ' );

        stream << indent << "coord:" << std::endl;
        for ( auto& c : coords )
        {
            if ( c )
            {
                c->print( stream, indent_count + 2 );
            }
            else
            {
                stream << sub_indent << "The previous line will be used" << std::endl;
            }
        }
        stream << indent << "variable:" << std::endl;
        for ( auto& v : variables )
        {
            if ( v )
            {
                v->print( stream, indent_count + 2 );
            }
            else
            {
                stream << sub_indent << "The previous line will be used" << std::endl;
            }
        }
    }
};

cvt::VtkXmlStructuredGrid GetVtkDataFromIrregularAvsField(
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field );
cvt::VtkXmlImageData GetVtkDataFromUniformAvsField(
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field );
cvt::VtkXmlRectilinearGrid GetVtkDataFromRectilinearAvsField(
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field );

template <typename T>
T GetVtkDataFromAvsField(
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field );

template <>
inline cvt::VtkXmlStructuredGrid GetVtkDataFromAvsField<cvt::VtkXmlStructuredGrid>(
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field )
{
    return cvt::detail::GetVtkDataFromIrregularAvsField( n, dirname, has_data_section, header,
                                                         time_dependent_field );
}

template <>
inline cvt::VtkXmlImageData GetVtkDataFromAvsField<cvt::VtkXmlImageData>(
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field )
{
    return cvt::detail::GetVtkDataFromUniformAvsField( n, dirname, has_data_section, header,
                                                       time_dependent_field );
}

template <>
inline cvt::VtkXmlRectilinearGrid GetVtkDataFromAvsField<cvt::VtkXmlRectilinearGrid>(
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field )
{
    return cvt::detail::GetVtkDataFromRectilinearAvsField( n, dirname, has_data_section, header,
                                                           time_dependent_field );
}

template <typename T>
class AvsFieldTimeSeriesIterator
{
public:
    AvsFieldTimeSeriesIterator(
        std::filesystem::path dirname, bool has_data_section,
        const cvt::detail::AvsFieldHeader& header,
        const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field, int c ):
        dirname( dirname ),
        has_data_section( has_data_section ),
        header( header ),
        time_dependent_field( time_dependent_field ),
        current( c )
    {
    }

public:
    T operator*() const
    {
        return cvt::detail::GetVtkDataFromAvsField<T>( current, dirname, has_data_section, header,
                                                       time_dependent_field );
    }
    cvt::detail::AvsFieldTimeSeriesIterator<T>& operator++()
    {
        ++current;
        return *this;
    }
    cvt::detail::AvsFieldTimeSeriesIterator<T> operator++( int )
    {
        auto prev = *this;
        ++current;
        return prev;
    }
    bool operator==( const cvt::detail::AvsFieldTimeSeriesIterator<T>& other ) const
    {
        return other.current == current;
    }
    bool operator!=( const cvt::detail::AvsFieldTimeSeriesIterator<T>& other ) const
    {
        return other.current != current;
    }

private:
    std::filesystem::path dirname;
    bool has_data_section;
    const cvt::detail::AvsFieldHeader& header;
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field;
    int current;
};

template <typename T>
class AvsFieldTimeSeriesContainer
{
public:
    AvsFieldTimeSeriesContainer(
        std::filesystem::path dirname, bool has_data_section,
        const cvt::detail::AvsFieldHeader& header,
        const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field ):
        dirname( dirname ),
        has_data_section( has_data_section ),
        header( header ),
        time_dependent_field( time_dependent_field )
    {
    }

public:
    AvsFieldTimeSeriesIterator<T> begin() const
    {
        return AvsFieldTimeSeriesIterator<T>( dirname, has_data_section, header,
                                              time_dependent_field, 0 );
    }
    AvsFieldTimeSeriesIterator<T> end() const
    {
        return AvsFieldTimeSeriesIterator<T>( dirname, has_data_section, header,
                                              time_dependent_field, header.nstep );
    }

private:
    std::filesystem::path dirname;
    bool has_data_section;
    const cvt::detail::AvsFieldHeader& header;
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field;
};
} // namespace detail
} // namespace cvt

namespace cvt
{

/**
 * An AVS Field file IO.
 */
class AvsField : public kvs::FileFormatBase
{
public:
    /**
     * A base class type.
     */
    typedef kvs::FileFormatBase BaseClass;

public:
    /**
     * Construct an IO.
     */
    AvsField(): kvs::FileFormatBase() {}
    /**
     * Construct an IO.
     *
     * \param[in] filename A file name.
     */
    AvsField( const std::string& filename ): BaseClass()
    {
        kvs::FileFormatBase::setFilename( filename );
        this->read( filename );
    }
    /**
     * Construct an IO.
     *
     * \param[in] filename A file name.
     */
    AvsField( std::string&& filename ): BaseClass()
    {
        kvs::FileFormatBase::setFilename( filename );
        this->read( filename );
    }

public:
    bool read( const std::string& filename ) override;
    bool write( const std::string& filename ) override
    {
        setSuccess( false );
        kvsMessageError( "This function has not been implemented yet" );
        return false;
    };

public:
    template <typename Stream>
    void print( Stream& stream, int indent_count = 0 )
    {
        header.print( stream );
        int time_step = 0;
        for ( auto& f : time_dependent_field )
        {
            stream << time_step++ << ":" << std::endl;
            f.print( stream, indent_count + 2 );
        }
    }
    /**
     * Check if this grid is irregular.
     *
     * \return `true` if this grid is irregular, otherwise `false`.
     */
    bool isIrregular() const { return header.field == "irregular"; }
    /**
     * Check if this grid is union.
     *
     * \return `true` if this grid is union , otherwise `false`.
     */
    bool isUnion() const { return header.field == "union"; }
    /**
     * Check if this grid is rectilinear.
     *
     * \return `true` if this grid is rectilinear, otherwise `false`.
     */
    bool isRectilinear() const { return header.field == "rectilinear"; }
    /**
     * Get a VTK StructuredGrid from an irregular grid.
     *
     * \return A file format.
     */
    cvt::VtkXmlStructuredGrid getAsIrregular( int n ) const
    {
        return cvt::detail::GetVtkDataFromAvsField<cvt::VtkXmlStructuredGrid>(
            n, dirname, has_data_section, header, time_dependent_field );
    }
    /**
     * Get a VTK UniformGrid from an uniform grid.
     *
     * \return A VTK UniformGrid.
     */
    cvt::VtkXmlImageData getAsUniform( int n ) const
    {
        return cvt::detail::GetVtkDataFromAvsField<cvt::VtkXmlImageData>(
            n, dirname, has_data_section, header, time_dependent_field );
    }
    /**
     * Get a VTK RectilinearGrid from an uniform grid.
     *
     * \return A VTK RectilinearGrid.
     */
    vtkSmartPointer<vtkRectilinearGrid> getAsRectilinear( int n ) const;
    /**
     * Get the last time step.
     *
     * This function does NOT return a time step count.
     * If there are 50 files for [0-49] time steps, this returns 49.
     *
     * \return The last time step. Zero-based numbering.
     */
    int lastTimeStep() const { return header.nstep - 1; }
    /**
     * Get an interface to iterate each time step file format.
     *
     * e.g.
     *
     * ```c++
     * cvt::AvsField avs;
     *
     * for (auto file_format : avs.eachTimeStepAsIrregular()) {
     *     // ...
     * }
     * ```
     *
     * \return An interface to iterate each time step file format.
     */
    cvt::detail::AvsFieldTimeSeriesContainer<cvt::VtkXmlStructuredGrid> eachTimeStepAsIrregular()
        const
    {
        return cvt::detail::AvsFieldTimeSeriesContainer<cvt::VtkXmlStructuredGrid>(
            dirname, has_data_section, header, time_dependent_field );
    }
    /**
     * Get an interface to iterate each time step file format.
     *
     * e.g.
     *
     * ```c++
     * cvt::AvsField avs;
     *
     * for (auto file_format : avs.eachTimeStepAsUnion()) {
     *     // ...
     * }
     * ```
     *
     * \return An interface to iterate each time step file format.
     */
    cvt::detail::AvsFieldTimeSeriesContainer<cvt::VtkXmlImageData> eachTimeStepAsUnion() const
    {
        return cvt::detail::AvsFieldTimeSeriesContainer<cvt::VtkXmlImageData>(
            dirname, has_data_section, header, time_dependent_field );
    }
    /**
     * Get an interface to iterate each time step file format.
     *
     * e.g.
     *
     * ```c++
     * cvt::AvsField avs;
     *
     * for (auto file_format : avs.eachTimeStepAsRectilinear()) {
     *     // ...
     * }
     * ```
     *
     * \return An interface to iterate each time step file format.
     */
    cvt::detail::AvsFieldTimeSeriesContainer<cvt::VtkXmlRectilinearGrid> eachTimeStepAsRectilinear()
        const
    {
        return cvt::detail::AvsFieldTimeSeriesContainer<cvt::VtkXmlRectilinearGrid>(
            dirname, has_data_section, header, time_dependent_field );
    }

private:
    std::filesystem::path dirname;
    std::shared_ptr<std::ifstream> fld;
    bool has_data_section;
    std::ifstream::pos_type data_section_head;
    cvt::detail::AvsFieldHeader header;
    std::vector<cvt::detail::AvsFieldTimeDependentField> time_dependent_field;
};
} // namespace cvt

#endif // CVT__AVS_FLD_H_INCLUDE
