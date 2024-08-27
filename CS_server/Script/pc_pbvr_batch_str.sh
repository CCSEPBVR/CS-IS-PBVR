#!/bin/bash

node_num=''
vin=''
div=''
ld=''
other_opt=''
thread_num="2"
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
        -d)
            div="$2"; shift;;
        -t)
            thread_num="$2"; shift;;
        # undefined option
        -ld)
            if [ -r "$2" ]
            then
                ld="$2"
            else
                echo 1>&2 " error: unreadable file -ld $2"
                exit 1
            fi
            shift;;
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
if [ -z $node_num ] || [ -z $vin ] || [ -z $div ] || [ -z $ld ]
then
    echo " usage: $0 -n node_num -t thread_num -vin pfi_file -pout output_dir -d div_num -ld loadmodule [PBVR option]"
    echo "     node_num: number of nodes per job"
    echo "     thread_num: number of threads"
    echo "     pfi_file: .pfi or .pfl file"
    echo "     div_num: divide number of timestep"
    echo "     loadmodule: PBVR server program"
    exit 1
fi

# parameter setting
all_node_num=$((node_num*div))

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

export OMP_NUM_THREADS=$thread_num
export PARALLEL=$thread_num

bs_array=(0 ${bs_array[@]})
be_array=(0 ${be_array[@]})

for i in \`seq 1 $div\`
do
  bs=\${bs_array[\$i]}
  be=\${be_array[\$i]}

  mpiexec -machinefile pbvr_machine_\$i -n $node_num $ld -B -vin $vin -Bs \$bs -Be \$be $other_opt &
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
echo "Prepair the following files before execute pbvr_script_$$.sh"
for i in `seq 1 $div`
do
    echo "  pbvr_machine_$i"
done


