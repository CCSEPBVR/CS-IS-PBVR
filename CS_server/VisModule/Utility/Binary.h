/****************************************************************************/
/**
 *  @file Binary.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Binary.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__BINARY_H_INCLUDE
#define VIS_MODULE__BINARY_H_INCLUDE

#define VIS_MODULE_BINARY_0000_TO_HEX 0
#define VIS_MODULE_BINARY_0001_TO_HEX 1
#define VIS_MODULE_BINARY_0010_TO_HEX 2
#define VIS_MODULE_BINARY_0011_TO_HEX 3
#define VIS_MODULE_BINARY_0100_TO_HEX 4
#define VIS_MODULE_BINARY_0101_TO_HEX 5
#define VIS_MODULE_BINARY_0110_TO_HEX 6
#define VIS_MODULE_BINARY_0111_TO_HEX 7
#define VIS_MODULE_BINARY_1000_TO_HEX 8
#define VIS_MODULE_BINARY_1001_TO_HEX 9
#define VIS_MODULE_BINARY_1010_TO_HEX A
#define VIS_MODULE_BINARY_1011_TO_HEX B
#define VIS_MODULE_BINARY_1100_TO_HEX C
#define VIS_MODULE_BINARY_1101_TO_HEX D
#define VIS_MODULE_BINARY_1110_TO_HEX E
#define VIS_MODULE_BINARY_1111_TO_HEX F

#define VIS_MODULE_HEX_DIGIT(a)  VIS_MODULE_BINARY_##a##_TO_HEX

#define VIS_MODULE_BINARY1H(a)               (0x##a)
#define VIS_MODULE_BINARY1I(a)               VIS_MODULE_BINARY1H(a)

#define VIS_MODULE_BINARY2H(a,b)             (0x##a##b)
#define VIS_MODULE_BINARY2I(a,b)             VIS_MODULE_BINARY2H(a,b)

#define VIS_MODULE_BINARY3H(a,b,c)           (0x##a##b##c)
#define VIS_MODULE_BINARY3I(a,b,c)           VIS_MODULE_BINARY3H(a,b,c)

#define VIS_MODULE_BINARY4H(a,b,c,d)         (0x##a##b##c##d)
#define VIS_MODULE_BINARY4I(a,b,c,d)         VIS_MODULE_BINARY4H(a,b,c,d)

#define VIS_MODULE_BINARY8H(a,b,c,d,e,f,g,h) (0x##a##b##c##d##e##f##g##h)
#define VIS_MODULE_BINARY8I(a,b,c,d,e,f,g,h) VIS_MODULE_BINARY8H(a,b,c,d,e,f,g,h)


#define visModuleBinary8(a,b) \
    VIS_MODULE_BINARY2I( VIS_MODULE_HEX_DIGIT(a), \
                  VIS_MODULE_HEX_DIGIT(b) )

#define visModuleBinary12(a,b,c) \
    VIS_MODULE_BINARY3I( VIS_MODULE_HEX_DIGIT(a), \
                  VIS_MODULE_HEX_DIGIT(b), \
                  VIS_MODULE_HEX_DIGIT(c) )

#define visModuleBinary16(a,b,c,d) \
    VIS_MODULE_BINARY4I( VIS_MODULE_HEX_DIGIT(a), \
                  VIS_MODULE_HEX_DIGIT(b), \
                  VIS_MODULE_HEX_DIGIT(c), \
                  VIS_MODULE_HEX_DIGIT(d) )

#define visModuleBinary32(a,b,c,d,e,f,g,h) \
    VIS_MODULE_BINARY8I( VIS_MODULE_HEX_DIGIT(a), \
                  VIS_MODULE_HEX_DIGIT(b), \
                  VIS_MODULE_HEX_DIGIT(c), \
                  VIS_MODULE_HEX_DIGIT(d), \
                  VIS_MODULE_HEX_DIGIT(e), \
                  VIS_MODULE_HEX_DIGIT(f), \
                  VIS_MODULE_HEX_DIGIT(g), \
                  VIS_MODULE_HEX_DIGIT(h) )

#endif // VIS_MODULE__BINARY_H_INCLUDE
