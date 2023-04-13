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
#ifndef CVT__VTK_TIME_SERIES_FILE_FORMAT_H_INCLUDE
#define CVT__VTK_TIME_SERIES_FILE_FORMAT_H_INCLUDE

#include <vtkGlobFileNames.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>

namespace cvt
{
namespace detail
{

template <typename FileFormat>
class NumeralSequenceFileIterator
{
public:
    NumeralSequenceFileIterator( vtkSmartPointer<vtkStringArray> filenames, int head = 0 ):
        filenames( filenames ), head( head )
    {
    }

public:
    FileFormat operator*() { return FileFormat( filenames->GetValue( head ) ); }
    FileFormat operator*() const { return FileFormat( filenames->GetValue( head ) ); }
    void operator++() noexcept { ++head; }
    NumeralSequenceFileIterator<FileFormat> operator++( int )
    {
        auto old = head;
        ++head;
        return NumeralSequenceFileIterator<FileFormat>( filenames, old );
    }
    bool operator!=( const NumeralSequenceFileIterator<FileFormat>& other )
    {
        return other.head != head;
    }
    bool operator!=( const NumeralSequenceFileIterator<FileFormat>& other ) const
    {
        return other.head != head;
    }
    bool operator==( const NumeralSequenceFileIterator<FileFormat>& other )
    {
        return other.head == head;
    }
    bool operator==( const NumeralSequenceFileIterator<FileFormat>& other ) const
    {
        return other.head == head;
    }

private:
    vtkSmartPointer<vtkStringArray> filenames;
    int head;
};

template <typename FileFormat>
class NumeralSequenceFileContainer
{
public:
    NumeralSequenceFileContainer( vtkSmartPointer<vtkStringArray> filenames ):
        filenames( filenames )
    {
    }

public:
    /**
     * Get a count of files which is matched a file name pattern.
     *
     * \return A count of files.
     */
    int numberOfFiles() { return filenames->GetNumberOfValues(); }
    /**
     * Get a count of files which is matched a file name pattern.
     *
     * \return A count of files.
     */
    int numberOfFiles() const { return filenames->GetNumberOfValues(); }
    /**
     * Return a file format iterator to the beginning.
     *
     * \return A file format iterator.
     */
    cvt::detail::NumeralSequenceFileIterator<FileFormat> begin()
    {
        return cvt::detail::NumeralSequenceFileIterator<FileFormat>( filenames );
    }
    /**
     * Return a file format iterator to the beginning.
     *
     * \return A file format iterator.
     */
    cvt::detail::NumeralSequenceFileIterator<FileFormat> begin() const
    {
        return cvt::detail::NumeralSequenceFileIterator<FileFormat>( filenames );
    }
    /**
     * Return a file format iterator to the end.
     *
     * \return A file format iterator.
     */
    cvt::detail::NumeralSequenceFileIterator<FileFormat> end()
    {
        return cvt::detail::NumeralSequenceFileIterator<FileFormat>(
            filenames, filenames->GetNumberOfValues() );
    }
    /**
     * Return a file format iterator to the end.
     *
     * \return A file format iterator.
     */
    cvt::detail::NumeralSequenceFileIterator<FileFormat> end() const
    {
        return cvt::detail::NumeralSequenceFileIterator<FileFormat>(
            filenames, filenames->GetNumberOfValues() );
    }

private:
    vtkSmartPointer<vtkStringArray> filenames;
};
} // namespace detail
} // namespace cvt

namespace cvt
{

/**
 * A FileFormat generator for files whose filename includes a numeral sequence.
 *
 * This class generates FileFormats from a file sequence like foo_0.vtk, foo_1.vtk, ..., foo_N.vtk.
 *
 * ```c++
 * std::string pattern = "foo_*.vtk";
 * cvt::NumeralSequenceFiles<SomeFileFormat> n( pattern );
 * int last_time_step = n.numberOfFiles() - 1;
 * for (SomeFileFormat f : n) {
 *     // ...
 * }
 * ```
 *
 * \tparam FileFormat A target file format type.
 */
template <typename FileFormat>
class NumeralSequenceFiles
{
public:
    /**
     * Construct a FileFormat generator.
     *
     * \param[in] pattern A file name pattern with wildcard `*` like `foo_*.vtk`.
     */
    NumeralSequenceFiles( const std::string& pattern )
    {
        vtkNew<vtkGlobFileNames> glob;
        glob->RecurseOff();
        glob->AddFileNames( pattern.c_str() );

        auto f = glob->GetFileNames();

        vtkNew<vtkSortFileNames> sorter;
        sorter->GroupingOff();
        sorter->NumericSortOn();
        sorter->IgnoreCaseOff();
        sorter->SkipDirectoriesOn();
        sorter->SetInputFileNames( f );

        filenames = sorter->GetFileNames();
    }

public:
    /**
     * Get a count of files which is matched a file name pattern.
     *
     * \return A count of files.
     */
    int numberOfFiles() { return filenames->GetNumberOfValues(); }
    /**
     * Get a count of files which is matched a file name pattern.
     *
     * \return A count of files.
     */
    int numberOfFiles() const { return filenames->GetNumberOfValues(); }
    /**
     * Get an interface to iterate time series files.
     *
     * \return An interface to iterate time series files.
     */
    cvt::detail::NumeralSequenceFileContainer<FileFormat> eachTimeStep()
    {
        return cvt::detail::NumeralSequenceFileContainer<FileFormat>( filenames );
    }
    /**
     * Get an interface to iterate time series files.
     *
     * \return An interface to iterate time series files.
     */
    cvt::detail::NumeralSequenceFileContainer<FileFormat> eachTimeStep() const
    {
        return cvt::detail::NumeralSequenceFileContainer<FileFormat>( filenames );
    }

private:
    vtkSmartPointer<vtkStringArray> filenames;
};
} // namespace cvt

#endif // CVT__VTK_TIME_SERIES_FILE_FORMAT_H_INCLUDE
