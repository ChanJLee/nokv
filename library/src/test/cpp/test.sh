make || exit $?

mkdir out
./main `pwd`/out demo.nkv

echo "test muti process"
#rm `pwd`/out/kv/mp.nkv
#./multi_process `pwd`/out mp.nkv