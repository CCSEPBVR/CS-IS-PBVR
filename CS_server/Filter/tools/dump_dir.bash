echo VTK CELL TYPE UNKNOWN
    od -tf4 -v -w12 $1/case_0000001_0000001_coord.dat
    od -i 	-v -w12 $1/case_0000001_0000001_connect.dat
echo VTK CELL TYPE 05 / UCD 2
    od -tf4 -v -w12 $1/02-case_0000001_0000001_coord.dat
    od -i 	-v -w12 $1/02-case_0000001_0000001_connect.dat
echo "      values"
    od -tf4 -v -w12 $1/02-case_00000_0000001_0000001_value.dat
echo VTK CELL TYPE 09 / UCD 3
    od -tf4 -v -w12 $1/03-case_0000001_0000001_coord.dat
    od -i 	-v -w16 $1/03-case_0000001_0000001_connect.dat
echo "      values"
    od -tf4 -v -w12 $1/03-case_00000_0000001_0000001_value.dat
