#!/bin/bash
prefix=${1}
pattern=(0 1 2 3 4 5 6 7)
wr=(0 0.1 0.5)
wr_str=("Read Only" "Read Heavy" "Write Heavy")
scale=${2}
for s in ${scale[@]}
do

    for p in ${pattern[@]}
    do 
        for w in ${wr[@]}
        do 
            echo ${p} $w ${s}
            ./generator ${p} $w ${s} > ${prefix}${s}_${p}_runtime_$w
            valgrind --tool=cachegrind ./generator ${p} $w ${s} > ${prefix}${s}_${p}_$w 2>&1
        done
    done
    i=0
    runtime_workload_string="${wr[$i]}:${wr_str[$i]}:runtime"
    echo $runtime_workload_string
    for ((i=1;i<${#wr[@]};i++))
    do
        str="${wr[$i]}:${wr_str[$i]}:runtime"
        runtime_workload_string="$runtime_workload_string,${str}"
    done
    i=0
    runtime_pattern_string="${prefix}${s}_${pattern[$i]}_runtime_"
    runtime_desc_string="P $i"
    for ((i=1;i<${#pattern[@]};i++))
    do
        str="${prefix}${s}_${pattern[$i]}_runtime_"
        runtime_pattern_string="$runtime_pattern_string,${str}"
        runtime_desc_string="$runtime_desc_string,P ${pattern[$i]}"
    done
    python parse_cachegrind.py "${s}_runtime.png" "Runtime" "$runtime_workload_string" "$runtime_pattern_string" "$runtime_desc_string"
    
    i=0
    missrate_workload_string="${wr[$i]}:${wr_str[$i]}:d1_read"
    for ((i=1;i<${#wr[@]};i++))
    do
        str="${wr[$i]}:${wr_str[$i]}:d1_overall"
        missrate_workload_string="$missrate_workload_string,${str}"
    done
    i=0
    missrate_pattern_string="${prefix}${s}_${pattern[$i]}_"
    missrate_desc_string="P $i"
    for ((i=1;i<${#pattern[@]};i++))
    do
        str="${prefix}${s}_${pattern[$i]}_"
        missrate_pattern_string="$missrate_pattern_string,${str}"
        missrate_desc_string="$missrate_desc_string,P $i"
    done


    python parse_cachegrind.py "${s}_cache.png" "L1 Cache Miss Rate (Normalized)" "$missrate_workload_string" "$missrate_pattern_string" "$missrate_desc_string"

done

