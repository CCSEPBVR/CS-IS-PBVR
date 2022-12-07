#!/bin/bash

node_num=''
vin=''
div=''
ld=''
other_opt=''
elapse_t=''
thread_num=2
mpi_num=''
que_name=''

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
        -d)
            div="$2"; shift;;
        -e)
            elapse_t="$2"; shift;;
        -t)
            thread_num="$2"; shift;;
        -m)
            mpi_num="$2"; shift;;
        -q)
            que_name="$2"; shift;;
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
            echo 1>&2 " error: unrecognized argumant $1"
            exit 1;;
    esac
    shift
done

# parameter check
if [ -z $node_num ] || [ -z $vin ] || [ -z $div ] || [ -z $ld ] || [ -z $elapse_t ] || [ -z $mpi_num ] || [ -z $que_name ]
then
    echo " usage: $0 -vin pfi_file -q que_name -n node_num -m proc_num -t thread_num -e elapse_time -d div_num -ld loadmodule [PBVR option]"
    echo "     pfi_file: .pfi or .pfl file"
    echo "     que_name: que resource name"
    echo "     node_num: number of nodes per job"
    echo "     proc_num: number of MPI process"
    echo "     thread_num: number of threads."
    echo "     elapse_time: job limit time"
    echo "     div_num: divide number of timestep"
    echo "     loadmodule: PBVR server program"
    exit 1
fi

# parameter set
ncpus=$((mpi_num*thread_num))
    
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

# divide timestep
ns=$((ts/div))
ns_remain=$((ts%div))

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
#PBS -q $que_name
#PBS -l select=${node_num}:ncpus=${ncpus}:mpiprocs=${mpi_num}:ompthreads=${thread_num}
#PBS -P PBVR
#PBS -l walltime=${elapse_t}
#PBS -J 1-$div

cd \$PBS_O_WORKDIR

. /etc/profile.d/modules.sh
module load intel/cur mpt/cur

bs_array=(0 ${bs_array[@]})
be_array=(0 ${be_array[@]})

mpijob $ld -B -vin $vin -Bs \${bs_array[\${PBS_ARRAY_INDEX}]} -Be \${be_array[\${PBS_ARRAY_INDEX}]} $other_opt
EOF

echo "generate pbvr_script_$$.sh"


