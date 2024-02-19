/****************************************************************************/
/**
 *  @file Argument.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Argument.h 626 2010-10-01 07:45:20Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVSMAKE__ARGUMENT_H_INCLUDE
#define KVSMAKE__ARGUMENT_H_INCLUDE

#include <kvs/CommandLine>


namespace kvsmake
{

/*==========================================================================*/
/**
 *  Argument class.
 */
/*==========================================================================*/
class Argument : public kvs::CommandLine
{
public:

    Argument( int argc, char** argv );
};

} // end of namespace kvsmake

#endif // KVSMAKE__ARGUMENT_H_INCLUDE
