/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__PARALLEL_VTK_FILE_FORMAT_H_INCLUDE
#define CVT__PARALLEL_VTK_FILE_FORMAT_H_INCLUDE
#include <iterator>
#include <string>

#include <vtkNew.h>
#include <vtkSmartPointer.h>

namespace cvt
{
namespace detail
{

template <typename ReaderPointer, typename FileFormatType>
class VtkPieceIterator
{
public:
    VtkPieceIterator( ReaderPointer r, int id = 0 ) noexcept: reader( r ), piece_id( id ) {}

public:
    FileFormatType operator*()
    {
        reader->UpdatePiece( piece_id, reader->GetNumberOfPieces(), -1 );
        auto output = reader->GetOutput();
        return FileFormatType( output );
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

template <typename PieceFileFormatType, typename VtkReaderType>
class VtkPieceContainer
{
public:
    VtkPieceContainer( int number_of_pieces, vtkSmartPointer<VtkReaderType> reader ):
        m_number_of_pieces( number_of_pieces ), reader( reader )
    {
    }

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

private:
    int m_number_of_pieces;
    vtkSmartPointer<VtkReaderType> reader;
};
} // namespace detail
} // namespace cvt

namespace cvt
{

/**
 * A parallel data file IO using VTK.
 *
 * A developer could read whole VTK data from `get()`.
 * Or a developer could read each piece VTK data from `eachPiece()`.
 */
template <typename PieceFileFormatType, typename VtkReaderType, typename VtkWriterType>
class ParallelVtkFileFormat
{
public:
    /**
     * Construct an IO object.
     *
     * \param [in] filename A file name.
     */
    ParallelVtkFileFormat( const std::string& filename )
    {
        reader = vtkSmartPointer<VtkReaderType>::New();
        reader->SetFileName( filename.c_str() );
        reader->UpdateInformation();
        m_number_of_pieces = reader->GetNumberOfPieces();
    }
    /**
     * Construct an IO object.
     *
     * \param [in] filename A file name.
     */
    ParallelVtkFileFormat( std::string&& filename )
    {
        reader = vtkSmartPointer<VtkReaderType>::New();
        reader->SetFileName( filename.c_str() );
        reader->UpdateInformation();
        m_number_of_pieces = reader->GetNumberOfPieces();
    }
    virtual ~ParallelVtkFileFormat() {}

public:
    /**
     * Get an interface to iterate each pieces (sub volumes).
     *
     * e.g.
     *
     *     SomeVtkPFileFormat file_format;
     *     for (auto piece_file_format : file_format.eachPiece()) {
     *         // ..
     *     }
     *
     * \return An interface to iterate each pieces.
     */
    cvt::detail::VtkPieceContainer<PieceFileFormatType, VtkReaderType> eachPiece()
    {
        return cvt::detail::VtkPieceContainer<PieceFileFormatType, VtkReaderType>(
            m_number_of_pieces, reader );
    }
    /**
     * Get an interface to iterate each pieces (sub volumes).
     *
     * e.g.
     *
     *     SomeVtkPFileFormat file_format;
     *     for (auto piece_file_format : file_format.eachPiece()) {
     *         // ..
     *     }
     *
     * \return An interface to iterate each pieces.
     */
    cvt::detail::VtkPieceContainer<PieceFileFormatType, VtkReaderType> eachPiece() const
    {
        return cvt::detail::VtkPieceContainer<PieceFileFormatType, VtkReaderType>(
            m_number_of_pieces, reader );
    }
    /**
     * Generate a file format to access whole data.
     *
     * \return A file format to access whole data.
     */
    PieceFileFormatType get()
    {
        reader->Update();
        auto output = reader->GetOutput();
        return PieceFileFormatType( output );
    }
    /**
     * Generate a file format to access whole data.
     *
     * \return A file format to access whole data.
     */
    PieceFileFormatType get() const
    {
        reader->Update();
        auto output = reader->GetOutput();
        return PieceFileFormatType( output );
    }

public:
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
    int m_number_of_pieces;
    vtkSmartPointer<VtkReaderType> reader;
};
} // namespace cvt

#endif // CVT__PARALLEL_VTK_FILE_FORMAT_H_INCLUDE
