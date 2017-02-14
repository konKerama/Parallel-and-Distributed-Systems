# A simple bash script that for running experiments
# Note: To run the script make sure it you have execution rights 
# (use: chmod u+x run_tests.sh to give execution rights) 
#!/bin/bash

NAME=$(hostname)
DATE=$(date "+%Y-%m-%d-%H:%M:%S")
FILE_PREF=$NAME-$DATE-test-tree

echo $NAME
echo $DATE

L=18
pop=128
rep=5

make clean; make
# run cube experiments
for n in $(seq 20 25)
do
  N=$(echo '2^'$n | bc)
    for i in $(seq 0 11)
    do
       T=$(echo '2^'$i | bc)
         echo "Running for 2^"$i" Threads"
         echo cube N=$N && ./test_octree $N $T 0 $pop $rep $L >> $FILE_PREF-cube.txt;
    done
done
# run octant experiments
for n in $(seq 20 25)
do
  N=$(echo '2^'$n | bc)
    for i in $(seq 0 11)
    do
       T=$(echo '2^'$i | bc)
         echo "Running for 2^"$i" Threads"
         echo plummer N=$N && ./test_octree $N $T 1 $pop $rep $L >> $FILE_PREF-plummer.txt;
    done
done













