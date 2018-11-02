../../../llvmBuild/bin/opt -dot-cfg ../test1.bc
for filename in *.dot; do
  dot -T png -O "$filename"
done
rm *.dot
