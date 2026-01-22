
OFFSET=0
FILE=$1

function getInteger {
    echo -n "$1" : 
    STRING=$( od  -vtd -w4 -Ad -N4 -j$OFFSET  $FILE |head -1)
    ((OFFSET+=4))
    echo $STRING
    arrIN=(${STRING//:/ })
    return ${arrIN[1]}
    
}
function getFloat {
    echo -n $1 : 
    od  -vtf4 -w4 -Ad -N4 -j$OFFSET $FILE  |head -1
    ((OFFSET+=4))
}

getInteger  'Num Nodes   '
getInteger  "Num elements"
getInteger  "Element_type"
getInteger  "file_type   "
getInteger  "out_num     "
getInteger  "m_nkinds    "
N_KINDS=$?
getInteger  "start_step  "
S_STEP=$?
getInteger  "end_step    "
E_STEP=$?
N_STEPS=$((E_STEP - S_STEP))
getInteger  "m_nvolumes  "
N_VOLS=$?
echo
echo XYZ Range
getFloat "min x     "
getFloat "min y     "
getFloat "min z     "
getFloat "max x     "
getFloat "max y     "
getFloat "max z     "
echo
echo m_nodes_per_subvolume[]
for i in `seq 1 ${N_VOLS}`;
do
    getInteger "volume ${i}"
done   
echo
echo m_elements_per_subvolume[]
for i in `seq 1 ${N_VOLS}`;
do
    getInteger "volume ${i}"
done
echo
echo m_tree[] range
for i in `seq 1 ${N_VOLS}`;
do
	echo m_tree[${i}]
	getFloat "min x     "l
	getFloat "min y     "
	getFloat "min z     "
	getFloat "max x     "
	getFloat "max y     "
	getFloat "max z     "
done
echo
echo step and m_kind[] range
for i in `seq 0 ${N_STEPS}`;
do
echo step ${i}
	for j in `seq 1 ${N_KINDS}`;
	do
		echo -n "    m_kind[${j}]"
		getFloat "    min value     "
		echo -n "    m_kind[${j}]"
		getFloat "    max value     "
	done
done
