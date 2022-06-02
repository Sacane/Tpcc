#!/bin/bash
make 


test(){
    n=0
    passed=0
    files=($1) 
    for f in ${files[@]}; do
        let n++
        printf "(%02d/%02d) - %s\n" $n ${#files[@]} "Testing $f..." 

        ./bin/tpcc < $f >> ./output.txt
        feedback=$PIPESTATUS

        echo $feedback >> ./test/feedback.txt ./output.txt 
        echo >> ./test/feedback.txt 

        if [ "$feedback" = "$2" ]; then
            let passed++
        fi
    done
    
    if [ $passed -eq ${#files[@]} ]; then
        echo -e "\033[0;32m$passed tests out of ${#files[@]} passed! \033[0;37m" 
        echo -e "$passed sem-error tests out of ${#files[@]} passed!" >> output.txt
    elif [ $passed = "0" ]; then
        echo -e "\033[0;31m$passed tests out of ${#files[@]} passed! \033[0;37m" 
        echo -e "$passed good tests out of ${#files[@]} passed!" >> output.txt
    else
        echo -e "\033[0;33m$passed tests out of ${#files[@]} passed! \033[0;37m" 
        echo -e "$passed syntaxic-tests tests out of ${#files[@]} passed!" >> output.txt
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