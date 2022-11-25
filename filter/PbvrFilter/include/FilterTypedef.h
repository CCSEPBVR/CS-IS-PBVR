/*
 * FilterTypedef.h
 *
 *  Created on: 2017/01/08
 *      Author: hira
 */

#ifndef FILTERTYPEDEF_H_
#define FILTERTYPEDEF_H_

// MSVC needs some help
#ifdef _MSC_VER
#define inline __inline
#define fseek _fseeki64
typedef  __int64	Int64;
#else
//#define Int64 long int
typedef long long    Int64;
#endif
typedef int    integer;
typedef float    real;

#endif /* FILTERTYPEDEF_H_ */
