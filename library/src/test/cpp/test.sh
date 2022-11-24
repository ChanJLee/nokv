make || exit $?

mkdir out
./main `pwd`/out/meta `pwd`/out/demo.nkv

echo "test muti process"
rm `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv
./multi_process `pwd`/out/meta `pwd`/out/mp.nkv