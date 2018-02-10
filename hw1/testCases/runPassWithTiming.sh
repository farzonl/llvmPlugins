./llvmJit.sh
echo -e "\n\n basicblock:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -basicblock -disable-output -time-passes test1.bc
echo -e "\n\n cfgedge:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -cfgedge -disable-output -time-passes test1.bc
echo -e "\n\n backedge:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -backedge -disable-output -time-passes test1.bc
