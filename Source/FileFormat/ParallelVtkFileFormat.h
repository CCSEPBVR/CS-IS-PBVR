/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__PARALLEL_VTK_FILE_FORMAT_H_INCLUDE
#define CVT__PARALLEL_VTK_FILE_FORMAT_H_INCLUDE
#include <algorithm>
#include <iterator>
#include <memory>
#include <string>

#include "kvs/File"
#include "kvs/FileFormatBase"
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include "VtkFileFormat.h"

namespace cvt
{
namespace detail
{
constexpr const char* const PIECE_MODE_GETTER_ERROR_MESSAGE =
    "a whole data was not ready in a piece-reader mode";
} // namespace detail
} // namespace cvt

namespace cvt
{

template <typename ReaderPointer, typename FileFormatType>
class VtkPieceIterator
{
public:
    VtkPieceIterator( ReaderPointer r, int id = 0 ) noexcept: reader( r ), piece_id( id ) {}

public:
    std::shared_ptr<FileFormatType> operator*()
    {
        reader->UpdatePiece( piece_id, reader->GetNumberOfPieces(), -1 );
        auto output = reader->GetOutput();
        return std::make_shared<FileFormatType>( output );
        // return nullptr;
    }
    void operator++() noexcept { ++piece_id; }
    VtkPieceIterator operator++( int )
    {
        auto old = *this;
        ++piece_id;
        return old;
    }
    void operator--() noexcept { --piece_id; }
    VtkPieceIterator operator--( int )
    {
        auto old = *this;
        --piece_id;
        return old;
    }
    bool operator==( const VtkPieceIterator& itr ) noexcept { return itr.piece_id == piece_id; }
    bool operator==( const VtkPieceIterator& itr ) const noexcept
    {
        return itr.piece_id == piece_id;
    }
    bool operator!=( const VtkPieceIterator& itr ) noexcept { return itr.piece_id != piece_id; }
    bool operator!=( const VtkPieceIterator& itr ) const noexcept
    {
        return itr.piece_id != piece_id;
    }

private:
    ReaderPointer reader;
    int piece_id;
};
} // namespace cvt

namespace cvt
{

/**
 * A parallel data file IO using VTK.
 *
 * This file format has two modes: a piece-reader or normal mode.
 *
 * In a piece-reader mode, this file format read only an data information when the construction.
 * After the construction, an user can read data pieces in a for loop.
 *
 * In a normal mode, this file format read whole data as same as others when the construction.
 */
template <typename PieceFileFormatType, typename VtkDataType_, typename VtkReaderType,
          typename VtkWriterType>
class ParallelVtkFileFormat : public cvt::VtkFileFormat<VtkDataType_, VtkReaderType, VtkWriterType>
{
public:
    /**
     * A base class type.
     */
    using BaseClass = cvt::VtkFileFormat<VtkDataType_, VtkReaderType, VtkWriterType>;
    /**
     * An inner VTK data type.
     */
    using VtkDataType = VtkDataType_;

public:
    /**
     * Construct an empty IO object.
     */
    ParallelVtkFileFormat() noexcept:
        kvs::FileFormatBase(), BaseClass(), m_is_piece_reader_mode( false ), m_number_of_pieces( 0 )
    {
    }
    /**
     * Construct an IO object.
     *
     * \param filename A file name.
     * \param is_piece_reader_mode A piece reader mode flag.
     */
    ParallelVtkFileFormat( const std::string& filename, bool is_piece_reader_mode = false ):
        BaseClass(), m_is_piece_reader_mode( is_piece_reader_mode )
    {
        if ( is_piece_reader_mode )
        {
            kvs::FileFormatBase::setFilename( filename );
            reader = vtkSmartPointer<VtkReaderType>::New();
            reader->SetFileName( filename.c_str() );
            reader->UpdateInformation();
            m_number_of_pieces = reader->GetNumberOfPieces();
        }
        else
        {
            BaseClass::read( filename );
            m_number_of_pieces = 1;
        }
    }
    /**
     * Construct an IO object.
     *
     * \param filename A file name.
     * \param is_piece_reader_mode A piece reader mode flag.
     */
    ParallelVtkFileFormat( std::string&& filename, bool is_piece_reader_mode = false ):
        BaseClass(), m_is_piece_reader_mode( is_piece_reader_mode )
    {
        if ( is_piece_reader_mode )
        {
            kvs::FileFormatBase::setFilename( filename );
            reader = vtkSmartPointer<VtkReaderType>::New();
            reader->SetFileName( filename.c_str() );
            reader->UpdateInformation();
            m_number_of_pieces = reader->GetNumberOfPieces();
        }
        else
        {
            BaseClass::read( filename );
            m_number_of_pieces = 1;
        }
    }
    /**
     * Construct an IO object.
     *
     * \param [in] vtk_data A VTK data.
     */
    ParallelVtkFileFormat( VtkDataType* data ):
        kvs::FileFormatBase(), BaseClass( data ), m_is_piece_reader_mode( false )
    {
    }
    virtual ~ParallelVtkFileFormat() {}

public:
    /**
     * Return a piece iterator to the beginning.
     *
     * \return A piece iterator.
     */
    VtkPieceIterator<vtkSmartPointer<VtkReaderType>, PieceFileFormatType> begin()
    {
        return VtkPieceIterator<vtkSmartPointer<VtkReaderType>, PieceFileFormatType>( reader );
    }
    /**
     * Return a piece iterator to the beginning.
     *
     * \return A piece iterator.
     */
    VtkPieceIterator<vtkSmartPointer<VtkReaderType>, PieceFileFormatType> begin() const
    {
        return VtkPieceIterator<vtkSmartPointer<VtkReaderType>, PieceFileFormatType>( reader );
    }
    /**
     * Return a piece iterator to the end.
     *
     * \return A piece iterator.
     */
    VtkPieceIterator<vtkSmartPointer<VtkReaderType>, PieceFileFormatType> end()
    {
        return VtkPieceIterator<vtkSmartPointer<VtkReaderType>, PieceFileFormatType>(
            reader, m_number_of_pieces );
    }
    /**
     * Return a piece iterator to the end.
     *
     * \return A piece iterator.
     */
    VtkPieceIterator<vtkSmartPointer<VtkReaderType>, PieceFileFormatType> end() const
    {
        return VtkPieceIterator<vtkSmartPointer<VtkReaderType>, PieceFileFormatType>(
            reader, m_number_of_pieces );
    }

public:
    vtkSmartPointer<VtkDataType> get()
    {
        if ( m_is_piece_reader_mode )
        {
            throw std::runtime_error( cvt::detail::PIECE_MODE_GETTER_ERROR_MESSAGE );
        }
        else
        {
            return BaseClass::get();
        }
    }
    vtkSmartPointer<VtkDataType> get() const
    {
        if ( m_is_piece_reader_mode )
        {
            throw std::runtime_error( cvt::detail::PIECE_MODE_GETTER_ERROR_MESSAGE );
        }
        else
        {
            return BaseClass::get();
        }
    }

public:
    /*
     * Get this file format is in a piece reader mode.
     *
     * \return true if this is in a piece mode.
     */
    bool is_piece_reader_mode() noexcept { return m_is_piece_reader_mode; }
    /*
     * Get this file format is in a piece reader mode.
     *
     * \return true if this is in a piece mode.
     */
    bool is_piece_reader_mode() const noexcept { return m_is_piece_reader_mode; }
    /**
     * Get a number of pieces.
     *
     * \return A number of pieces.
     */
    int number_of_pieces() noexcept { return m_number_of_pieces; }
    /**
     * Get a number of pieces.
     *
     * \return A number of pieces.
     */
    int number_of_pieces() const noexcept { return m_number_of_pieces; }

private:
    bool m_is_piece_reader_mode;
    int m_number_of_pieces;
    vtkSmartPointer<VtkReaderType> reader;
};
} // namespace cvt

#endif // CVT__PARALLEL_VTK_FILE_FORMAT_H_INCLUDE
