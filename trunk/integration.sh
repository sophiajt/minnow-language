if [ -d tmp ]
then 
	rm -rf tmp
fi
mkdir tmp

./bin/minnowc -O3 samples/obj6.mno -o tmp/obj6
echo "Running obj6..."
output=$(./tmp/obj6)
if [ $output != "3Hello" ]; then
  echo "Wrong output: $output"
  kill 0
fi

./bin/minnowc -O3 samples/obj7.mno -o tmp/obj7
echo "Running obj7..."
output=$(./tmp/obj7)
if [ $output != "3Hello" ]; then
  echo "Wrong output: $output"
  kill 0
fi

echo "Running big bang..."
./bin/minnowc -O3 samples/big_bang.mno -o tmp/big_bang
echo "500"
time ./tmp/big_bang 500
OUT=$?
if [ $OUT -ne 0 ]; then
  kill 0
fi

echo "1500"
time ./tmp/big_bang 1500
OUT=$?
if [ $OUT -ne 0 ]; then
  kill 0
fi

echo "2500"
time ./tmp/big_bang 2500
OUT=$?
if [ $OUT -ne 0 ]; then
  kill 0
fi

echo "Running threadring..."
./bin/minnowc -O3 samples/threadring.mno -o tmp/threadring

echo "10000000"
time ./tmp/threadring 10000000
OUT=$?
if [ $OUT -ne 0 ]; then
  kill 0
fi

echo "50000000"
time ./tmp/threadring 50000000
OUT=$?
if [ $OUT -ne 0 ]; then
  kill 0
fi

echo "Running rebalance..."
./bin/minnowc -O3 samples/rebalance.mno -o tmp/rebalance
echo "1000 1000000"
time ./tmp/rebalance 1000 1000000
OUT=$?
if [ $OUT -ne 0 ]; then
  kill 0
fi

echo "100 10000000"
time ./tmp/rebalance 100 10000000
OUT=$?
if [ $OUT -ne 0 ]; then
  kill 0
fi

