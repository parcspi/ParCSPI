




clang++ -emit-llvm -c  $PAC_DIR/log_lib/log_lib.c -o $PAC_DIR/log_lib/log_lib.bc
clang++ -emit-llvm -c  $PAC_DIR/log_lib/pac_check_lib.c -o $PAC_DIR/log_lib/pac_check_lib.bc