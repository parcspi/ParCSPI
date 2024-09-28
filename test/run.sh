POLICY_SECURITY_SCORE=0.7
current_path=$(pwd)
if [ -d "collected_data" ]; then
    rm -r collected_data
fi
mkdir collected_data

if [ -d "collected_BTI_data" ]; then
    rm -r collected_BTI_data
fi
mkdir collected_BTI_data


TEST_DIR=$PAC_DIR/test/test_target

additionalFlag=" -lm "

# $TEST_DIR

# Iterate over all folders under $PAC_DIR/eval
for folder in "$TEST_DIR/eval/evaluation/src"/*; do
    if [ -d "$folder" ]; then
        # Perform actions on each folder
        echo "Processing folder: $folder"
        
        # folder=$PAC_DIR/eval/evaluation/src/wikisort
        bm_name=$(basename "$folder")
        outfile=$folder/binary.bc.opt
        test_file=$folder/single.bc
        inst_file=$folder/binary.inst.bc
        cd $folder
        rm *.ll *.bc *.txt pacgen_main.c demorun
       
        clang -c -emit-llvm -S *.c
        llvm-link *.ll -o $test_file
        opt -f -enable-new-pm=0 -load $SVF_DIR/Release-build/tools/PAC_ANALYSIS/libpac_analysis.so -pac_analysisPass $test_file -o $outfile
        python3 $PAC_DIR/bench_eval/gene_main.py $folder $bm_name
        clang -c -emit-llvm -S pacgen_main.c -o pacgen_main.ll
        clang++ -flto $additionalFlag pacgen_main.ll $outfile $PAC_DIR/log_lib/log_lib.bc -o demorun
        
        if [ -d "trace" ]; then
            rm -r trace
        fi
        mkdir trace

        ./demorun
        python3 $PAC_DIR/data_analysis/main.py $folder/pac_log_str_pt.txt $folder/trace/output.txt1 \
         $TEST_DIR/eval/collected_realdata/collected_timer_data/timer_data.$bm_name.txt $POLICY_SECURITY_SCORE optimal_policy.txt
        opt -f -enable-new-pm=0 -load $SVF_DIR/Release-build/tools/PAC_ENFORCE/libpac_enforce.so -pac_enforcePass $outfile -o $inst_file
         
        clang++ -flto $additionalFlag $inst_file pacgen_main.ll $PAC_DIR/log_lib/pac_check_lib.bc -o protected_bin
        ./protected_bin
        echo "Folder name: $folder"

        # Add your code here to perform actions on each folder
        # break
    fi
done

cd $current_path
