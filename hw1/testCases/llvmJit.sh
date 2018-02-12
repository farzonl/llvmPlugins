../../llvmBuild/bin/clang -emit-llvm test1.c -c -o test1.bc
../../llvmBuild/bin/lli test1.bc
../../llvmBuild/bin/clang -emit-llvm test2.c  -c -o test2.bc
../../llvmBuild/bin/lli test2.bc
../../llvmBuild/bin/clang -emit-llvm test3.c  -c -o test3.bc
../../llvmBuild/bin/lli test3.bc
