./llvmJit.sh
echo -e "\n\n basicblock:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -basicblock -disable-output -time-passes test1.bc
echo -e "\n\n cfgedge:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -cfgedge -disable-output -time-passes test1.bc
echo -e "\n\n backedge:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -backedge -disable-output -time-passes test1.bc
echo -e "\n\n loopbasicblock:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -loopbasicblock -disable-output -time-passes test1.bc
echo -e "\n\n dominatorspass:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -dominatorspass -disable-output -time-passes test1.bc
echo -e "\n\n propdompass:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -propdompass -disable-output -time-passes test1.bc
echo -e "\n\n allloops:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -allloops -disable-output -time-passes test1.bc
echo -e "\n\n toploops:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -toploops -disable-output -time-passes test1.bc
echo -e "\n\n exitcfgloops:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -exitcfgloops -disable-output -time-passes test1.bc
echo -e "\n\n warshloopdetector:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -warshloopdetector -disable-output -time-passes test1.bc
echo -e "\n\n controldep:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -controldep -disable-output -time-passes test1.bc
echo -e "\n\n reachable:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -reachable -disable-output -time-passes test1.bc
echo -e "\n"