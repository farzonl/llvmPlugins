./llvmJit.sh
echo -e "\n\n basicblock:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -basicblock < test1.bc > /dev/null
echo -e "\n\n cfgedge:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -cfgedge < test1.bc > /dev/null
echo -e "\n\n backedge:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -backedge < test1.bc > /dev/null
echo -e "\n\n loopbasicblock:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -loopbasicblock < test1.bc > /dev/null
echo -e "\n\n dominatorspass:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -dominatorspass < test1.bc > /dev/null
echo -e "\n\n allloops:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -allloops < test1.bc > /dev/null
echo -e "\n\n toploops:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -toploops < test1.bc > /dev/null
echo -e "\n\n exitcfgloops:"
../../llvmBuild/bin/opt -load ../../llvmBuild/lib/LLVMBackEdges.dylib -exitcfgloops < test1.bc > /dev/null
echo -e "\n"