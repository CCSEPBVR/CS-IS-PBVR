/****************************************************************************/
/**
 *  @file MarchingTetrahedraTable.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MarchingTetrahedraTable.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "MarchingTetrahedraTable.h"


namespace kvs
{

namespace MarchingTetrahedraTable
{

const int TriangleID[16][7] =
{
    { -1,-1,-1,-1,-1,-1,-1 },
    {  2, 1, 0,-1,-1,-1,-1 },
    {  0, 3, 5,-1,-1,-1,-1 },
    {  2, 1, 3, 3, 5, 2,-1 },
    {  4, 3, 1,-1,-1,-1,-1 },
    {  0, 2, 4, 4, 3, 0,-1 },
    {  1, 4, 5, 5, 0, 1,-1 },
    {  4, 5, 2,-1,-1,-1,-1 },

    {  2, 5, 4,-1,-1,-1,-1 },
    {  1, 0, 5, 5, 4, 1,-1 },
    {  0, 3, 4, 4, 2, 0,-1 },
    {  1, 3, 4,-1,-1,-1,-1 },
    {  2, 5, 3, 3, 1, 2,-1 },
    {  5, 3, 0,-1,-1,-1,-1 },
    {  0, 1, 2,-1,-1,-1,-1 },
    { -1,-1,-1,-1,-1,-1,-1 }
};

const int VertexID[6][2] =
{
    { 0, 1 },
    { 0, 2 },
    { 0, 3 },
    { 1, 2 },
    { 2, 3 },
    { 3, 1 }
};

} // end of namespace MarchingTetrahedraTable

} // end of namespace kvs
