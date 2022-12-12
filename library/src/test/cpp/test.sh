make || exit $?

mkdir out
./main `pwd`/out demo

#echo "test muti process"
#rm `pwd`/out/mp.nokv
#ps | grep multi_process | awk '{print $1}' | xargs -I {} kill -9 {}
#./multi_process `pwd`/out mp


#4 + 8  + 1 + 1 = 14 boolean
#4 + 7 + 1 + 4 + 13 = 29 > 43 string
#4 + 6 + 1 + 4 = 15 > 58 float
#4 + 6 + 1 + 4 = 15 > 73 int32
#4 + 6 + 1 + 8 = 19 > 92 int64
#4 + 7 + 1 + 4 + 5 = 21 > 113 suffix
#4 + 7 + 1 + 4 + 12 = 28 > 141 string
#113 - 29 = 84