#!/bin/bash

node_num=''
vin=''
pout=''
div=''
ld=''
elapse_t=''
other_opt=''
thread_num=8
pa=''

# option check
while [ "$1" != '' ]
do
    case "$1" in
        -n)
            node_num="$2"; shift;;
        -vin)
            if [ -a "$2" ]
            then
                vin="$2"
            else
                echo 1>&2 " error: $2 is not found."
                exit 1
            fi
            shift;;
        -pout)
            pout="$2"; shift;;
        -pa)
            if [ -a "$2" ]
            then
                pa="$2"
            else
                echo 1>&2 " error: $2 is not found."
                exit 1
            fi
            shift;;
        -d)
            div="$2"; shift;;
        -e)
            elapse_t="$2"; shift;;
        -t)
            thread_num="$2"; shift;;
        -ld)
            if [ -r "$2" ]
            then
                ld="$2"
            else
                echo 1>&2 " error: unreadable file -ld $2"
                exit 1
            fi
            shift;;
        # undefined option
        -*)
#            echo 1>&2 " error: undefined option $1"
#            exit 1;;
            case "$2" in
                -*)
                    other_opt="$other_opt $1";;
                *)
                    other_opt="$other_opt $1 $2"; shift;;
            esac;;

        *)
            echo 1>&2 " error: unrecognized argument $1"
            exit 1;;
    esac
    shift
done

# parameter check
if [ -z $node_num ] || [ -z $vin ] || [ -z $pout ] || [ -z $div ] || [ -z $ld ] || [ -z $elapse_t ]
then
    echo " usage: $0 -n node_num -t thread_num -e elapse_time -vin pfi_file -pout output_dir -d div_num -ld loadmodule [PBVR options]"
    echo "     node_num: number of nodes per job"
    echo "     thread_num: number of threads"
    echo "     elapse_time: job limit time"
    echo "     pfi_file: .pfi or .pfl file"
    echo "     output_dir: output directory name"
    echo "     div_num: divide number of timestep"
    echo "     loadmodule: PBVR server program"
    exit 1
fi

if [ $div -lt 1 ] || [ $div -gt 128 ]
then
    echo " error: -d div_num. div_num must be 0 < div_num < 129."
    exit 1
fi

# parameter setting
all_node_num=$((node_num*div))
if [ $all_node_num -le 384 ]; then
    rscgrp="small"
elif [ $all_node_num -gt 384 ] && [ $all_node_num -le 36864 ]; then
    rscgrp="large"
else
    rscgrp="huge"
fi

data_dir=`dirname $vin`
tt=`echo $data_dir | cut -b 1`
if [ ! $tt == '.' ] && [ ! $tt == '/' ]
then
    data_dir="./"$data_dir
fi

pfi=`basename $vin`

tt=`echo $pout | cut -b 1`
if [ $tt == '.' ] || [ $tt == '/' ]
then
    output_dir=$pout
else
    output_dir="./"$pout
fi
output_prefix=`basename $output_dir`
output_dir=`dirname $output_dir`

pa_stgin=''
pa_opt=''
if [ $pa != '' ]; then
    tt=`echo $pa | cut -b 1`
    if [ $tt == '.' ] || [ $tt == '/' ]
    then
        pa=$pa
    else
        pa="./"$pa
    fi
    pa_file=`basename $pa`
    pa_stgin="#PJM --stgin \"$pa ./$pa_file\""
    pa_opt="-pa ./$pa_file"
fi

# get timestep range
pfi_info=(`pfinfo.py $vin`)
if [ $? -gt 0 ]; then
    exit 1
fi
bs=${pfi_info[0]}
be=${pfi_info[1]}
ts=$((be-bs+1))

if [ $div -ge $ts ]
then
    div=$ts
fi

# generate vcoordfile & divide timestep
ns=$((ts/div))
ns_remain=$((ts%div))

nid=0
tt=$bs
for i in `seq 1 $div`
do
    if [ $ns_remain -gt 0 ]
    then
        tsb=$((ns+1))
        ns_remain=$((ns_remain-1))
    else
        tsb=$ns
    fi
    rm -f pbvr_vcoordfile_$$_$i
    for j in `seq 1 $node_num`
    do
        echo "($nid)" >> pbvr_vcoordfile_$$_$i
        nid=$((nid+1))
    done
    bs_array[$i]=$tt
    be_array[$i]=$((tt+tsb-1))
    tt=$((tt+tsb))
#    echo " div $i: timestep range = ${bs_array[$i]} - ${be_array[$i]}"
done

#echo "${bs_array[@]}"

rm -f pbvr_script_$$.sh

cat<<EOF > pbvr_script_$$.sh
#!/bin/bash -x
#
#PJM --rsc-list "node=$all_node_num"
#PJM --rsc-list "rscgrp=$rscgrp"
#PJM --rsc-list "elapse=$elapse_t"
#PJM --stg-transfiles all
#PJM --stgin "$ld ./a.out"
#PJM --stgin "./pbvr_vcoordfile_$$_* ./"
#PJM --stgin "$data_dir/* ./input_data/"
$pa_stgin
#PJM --stgout "./output_data/* $output_dir/"
#PJM -s

. /work/system/Env_base

export OMP_NUM_THREADS=$thread_num
export PARALLEL=$thread_num

mpiexec /work/system/bin/msh "/bin/mkdir -p output_data"

bs_array=(0 ${bs_array[@]})
be_array=(0 ${be_array[@]})

for i in \`seq 1 $div\`
do
  bs=\${bs_array[\$i]}
  be=\${be_array[\$i]}

  mpiexec --vcoordfile pbvr_vcoordfile_$$_\$i -n $node_num ./a.out -B -vin input_data/$pfi -pout output_data/$output_prefix $pa_opt -Bs \$bs -Be \$be $other_opt &
  eval pid_\$i=\$!
  eval echo '\$pid_'\$i
done

for i in \`seq 1 $div\`
do
  eval wait '\$pid_'\$i
  echo "job \$i done. code: \$?"
done
EOF

echo "generate pbvr_script_$$.sh"


