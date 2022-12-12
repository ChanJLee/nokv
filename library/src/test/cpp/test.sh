make || exit $?

mkdir out
./main `pwd`/out demo

#echo "test muti process"
#rm `pwd`/out/mp.nokv
#ps | grep multi_process | awk '{print $1}' | xargs -I {} kill -9 {}
#./multi_process `pwd`/out mp