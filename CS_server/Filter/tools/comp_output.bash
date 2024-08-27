SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
for file1 in $1/*.dat
do
    file2="${file1/$1/$2}"
    base="${file1/$1/}"
    cmp ${file1} ${file2}
    if [ $? -eq 0 ]; then
	echo -e "${base} is the same in both dirs "
    fi
done
for file1 in $1/*.pfi
do
    echo Dumping .pfi Files
    file2="${file1/$1/$2}"
    ${SCRIPT_DIR}/dump_pfi.bash ${file1} >${file1}.dump 
    ${SCRIPT_DIR}/dump_pfi.bash ${file2} >${file2}.dump 
    echo  "Running diff on  pfi dumps."
    echo -n "Number of pfi entries that differ:"
    diff -y --suppress-common-lines ${file1}.dump ${file2}.dump |wc -l
    diff -y --suppress-common-lines ${file1}.dump ${file2}.dump  >${file1}.diff
    diff -y --suppress-common-lines ${file2}.dump ${file1}.dump  >${file2}.diff
    echo "See ${file1}.diff and ${file2}.diff for more information"

done
