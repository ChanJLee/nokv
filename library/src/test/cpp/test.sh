make || exit $?

mkdir out
./main `pwd`/out demo

echo "test muti process"
rm `pwd`/out/mp.*
./multi_process `pwd`/out mp