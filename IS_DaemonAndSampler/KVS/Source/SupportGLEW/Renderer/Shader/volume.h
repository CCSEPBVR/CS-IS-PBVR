/*****************************************************************************/
/**
 *  @file   volume.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: volume.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
struct Volume
{
    vec3 resolution; // volume resolution
    vec3 resolution_ratio; // ratio of the resolution (256x256x128 => 1:1:0.5)
    vec3 resolution_reciprocal; // reciprocal number of the resolution
    float min_range; // min. range of the value
    float max_range; // max. range of the value
    sampler3D data; // volume data (scalar field)
};
