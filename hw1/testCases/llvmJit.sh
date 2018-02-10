clang -emit-llvm test1.c -c -o test1.bc
../../llvmBuild/bin/lli test1.bc
