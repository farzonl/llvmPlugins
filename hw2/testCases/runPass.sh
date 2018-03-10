./llvmJit.sh
echo -e "\n\n nuninit:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/uninitVars.dylib -nuninit < test1.bc > /dev/null
 echo -e "\n"
 echo -e "\n\n runinit:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/uninitVars.dylib -runinit < test1.bc > /dev/null
echo -e "\n"
