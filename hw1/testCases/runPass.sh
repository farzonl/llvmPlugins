./llvmJit.sh
echo -e "\n\n basicblock:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -basicblock < test1.bc > /dev/null
echo -e "\n\n cfgedge:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -cfgedge < test1.bc > /dev/null
echo -e "\n\n backedge:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -backedge < test1.bc > /dev/null
