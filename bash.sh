#!/bin/bash
make 


test(){
    n=0
    passed=0
    files=($1) #$1 $2 args
    for f in ${files[@]}; do
        let n++
        printf "(%02d/%02d) - %s\n" $n ${#files[@]} "Testing $f..." 

        ./bin/tpcc < $f >> ./test/feedback.txt
        feedback=$PIPESTATUS

        echo $feedback >> ./test/feedback.txt
        echo >> ./test/feedback.txt

        if [ "$feedback" = "$2" ]; then
            let passed++
        fi
    done
    
    if [ $passed -eq ${#files[@]} ]; then
        echo -e "\033[0;32m$passed tests out of ${#files[@]} passed! \033[0;37m"
    elif [ $passed = "0" ]; then
        echo -e "\033[0;31m$passed tests out of ${#files[@]} passed! \033[0;37m"
    else
        echo -e "\033[0;33m$passed tests out of ${#files[@]} passed! \033[0;37m"
    fi
}

echo "=============== Valid tests  ===============" 
test "./test/good/*.tpc" 0

echo "============== Invalid tests ===============" 
test "./test/syn-err/*.tpc" 1

echo "============== Invalid tests ===============" 
test "./test/sem-err/*.tpc" 2

echo "Done."

make clean