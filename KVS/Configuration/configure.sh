#!/bin/bash

#=============================================================================
#  Configure BuildRule.
#=============================================================================
printf 'configure BuildRule... '

./configure_BuildRule.py Core
./configure_BuildRule.py SupportGLUT
./configure_BuildRule.py SupportGLEW
./configure_BuildRule.py SupportOpenCV
./configure_BuildRule.py SupportSAGE
./configure_BuildRule.py SupportCUDA
./configure_BuildRule.py SupportOpenCABIN

printf 'success\n'


#=============================================================================
#  Configure KVS_HEADER_LIST.
#=============================================================================
printf 'configure KVS_HEADER_LIST... '

for FILENAME in $( find .. -name 'KVS_HEADER_LIST' ); do
    sort -u -o ${FILENAME} ${FILENAME}
done

printf 'success\n'


#=============================================================================
#  Configure header list.
#=============================================================================
printf 'configure header list.. '

#rm -f $( find ../Source/kvs -type f -not -path '*/CVS/*' )
rm -f $( find ../Source/kvs -type f -not -path '*/.svn/*' )

./configure_header_list.py Core             kvs
./configure_header_list.py SupportGLUT      kvs/glut
./configure_header_list.py SupportGLEW      kvs/glew
./configure_header_list.py SupportSAGE      kvs/sage
./configure_header_list.py SupportOpenCV    kvs/opencv
./configure_header_list.py SupportQt        kvs/qt
./configure_header_list.py SupportCUDA      kvs/cuda
./configure_header_list.py SupportOpenCABIN kvs/opencabin

printf 'success\n'


#=============================================================================
#  Configure permission.
#=============================================================================
printf 'configure permission... '

#chmod 644 $( grep -I -l '' $( find .. -type f -not -path '*/CVS/*' ) )
chmod 644 $( grep -I -l '' $( find .. -type f -not -path '*/.svn/*' ) )
chmod 755 $( find .. -type d )
chmod 755 $( find .. -name '*.sh' -o -name '*.py')

printf 'success\n'
