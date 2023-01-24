/*
 * Copyright (c) 2022 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CVT__PARALLEL_VTK_FILE_FORMAT_H_INCLUDE
#define CVT__PARALLEL_VTK_FILE_FORMAT_H_INCLUDE
#include <iterator>
#include <string>

#include <vtkNew.h>
#include <vtkPCellDataToPointData.h>
#include <vtkRemoveGhosts.h>
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
        auto piece = dynamic_cast<typename FileFormatType::VtkDataType*>( reader->GetOutput() );

        vtkNew<vtkPCellDataToPointData> point_data_sampler;
        point_data_sampler->SetInputData( piece );
        vtkNew<vtkRemoveGhosts> ghost_remover;
        ghost_remover->SetInputConnection( point_data_sampler->GetOutputPort() );
        ghost_remover->Update();
        auto output =
            dynamic_cast<typename FileFormatType::VtkDataType*>( ghost_remover->GetOutput() );

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
 *
 * \tparam PieceFileFormatType A piece VTK file format type.
 * \tparam VtkReaderType A VTK reader type.
 * \tparam VtkWriterType A VTK writer type.
 */
template <typename PieceFileFormatType, typename VtkReaderType, typename VtkWriterType>
class ParallelVtkFileFormat
{
public:
    /**
     * Construct an IO object.
     *
     * \param[in] filename A file name.
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
     * \param[in] filename A file name.
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
     * ```c++
     * SomeVtkPFileFormat file_format;
     * for (auto piece_file_format : file_format.eachPiece()) {
     *     // ..
     * }
     * ```
     *
     * \return An interface to iterate each piece.
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
     * ```c++
     * SomeVtkPFileFormat file_format;
     * for (auto piece_file_format : file_format.eachPiece()) {
     *     // ..
     * }
     * ```
     *
     * \return An interface to iterate each piece.
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
        using VtkDataType = typename PieceFileFormatType::VtkDataType;

        if ( std::is_base_of_v<vtkPolyData, VtkDataType> ||
             std::is_base_of_v<vtkUnstructuredGrid, VtkDataType> )
        {
            vtkNew<vtkPCellDataToPointData> point_data_sampler;
            point_data_sampler->SetInputConnection( reader->GetOutputPort() );
            vtkNew<vtkRemoveGhosts> ghost_remover;
            ghost_remover->SetInputConnection( point_data_sampler->GetOutputPort() );
            ghost_remover->Update();

            return PieceFileFormatType( dynamic_cast<VtkDataType*>( ghost_remover->GetOutput() ) );
        }
        else
        {
            vtkNew<vtkPCellDataToPointData> point_data_sampler;
            point_data_sampler->SetInputConnection( reader->GetOutputPort() );

            point_data_sampler->Update();

            return PieceFileFormatType(
                dynamic_cast<VtkDataType*>( point_data_sampler->GetOutput() ) );
        }
    }
    /**
     * Generate a file format to access whole data.
     *
     * \return A file format to access whole data.
     */
    PieceFileFormatType get() const
    {
        using VtkDataType = typename PieceFileFormatType::VtkDataType;

        if ( std::is_base_of_v<vtkPolyData, VtkDataType> ||
             std::is_base_of_v<vtkUnstructuredGrid, VtkDataType> )
        {
            vtkNew<vtkPCellDataToPointData> point_data_sampler;
            point_data_sampler->SetInputConnection( reader->GetOutputPort() );
            vtkNew<vtkRemoveGhosts> ghost_remover;
            ghost_remover->SetInputConnection( point_data_sampler->GetOutputPort() );
            ghost_remover->Update();

            return PieceFileFormatType( dynamic_cast<VtkDataType*>( ghost_remover->GetOutput() ) );
        }
        else
        {
            vtkNew<vtkPCellDataToPointData> point_data_sampler;
            point_data_sampler->SetInputConnection( reader->GetOutputPort() );

            point_data_sampler->Update();

            return PieceFileFormatType(
                dynamic_cast<VtkDataType*>( point_data_sampler->GetOutput() ) );
        }
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
