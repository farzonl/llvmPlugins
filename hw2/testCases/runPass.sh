./llvmJit.sh
echo -e "\n\n uninit:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/uninitVars.dylib -uninit < test1.bc > /dev/null
 echo -e "\n"
