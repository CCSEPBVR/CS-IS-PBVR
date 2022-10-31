/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE
#define CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE

#include "kvs/FileFormatBase"

namespace cvt
{

/**
 * A base class for a composite data set IO.
 */
template <typename FileFormatIterator>
class CompositeDataSetFormatBase : public kvs::FileFormatBase
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
    CompositeDataSetFormatBase(): kvs::FileFormatBase() {}
    ~CompositeDataSetFormatBase() {}

public:
    virtual bool read( const std::string& filename ) = 0;
    virtual bool write( const std::string& filename ) = 0;
    /**
     * Return an iterator to the beginning of a composite data set leaf.
     *
     * \return an iterator.
     */
    virtual FileFormatIterator begin() = 0;
    /**
     * Return an iterator to the end of a composite data set leaf.
     *
     * \return an iterator.
     */
    virtual FileFormatIterator end() = 0;
    /**
     * Return an iterator to the beginning of a composite data set leaf.
     *
     * \return an iterator.
     */
    virtual FileFormatIterator cbegin() const = 0;
    /**
     * Return an iterator to the end of a composite data set leaf.
     *
     * \return an iterator.
     */
    virtual FileFormatIterator cend() const = 0;
};
} // namespace cvt

#endif // CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE