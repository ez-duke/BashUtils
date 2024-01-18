FUNC_SUCCESS=0
FUNC_FAIL=0
COUNTER=0
DIFF=""

declare -a flags=(
"i"
"v"
"c"
"l"
"n"
"h"
"s"
"o"
)

declare -a files=(
"s21_greptest1.txt s21_greptest2.txt"
#"s21_greptest1.txt"
)

declare -a commline=(
"OPTIONS FILE"
)

function testing()
{
    str=$(echo $@ | sed "s/OPTIONS/$options/")
    str=$(echo $str | sed -e "s/FILE/$file/g")
    ./s21_grep $str > s21_grep_testing.log
    grep $str > system_grep_testing.log
    DIFF="$(diff -s s21_grep_testing.log system_grep_testing.log)"
    (( COUNTER++ ))
    if [ "$DIFF" == "Files s21_grep_testing.log and system_grep_testing.log are identical" ]
    then
        (( FUNC_SUCCESS++ ))
        echo "TEST $COUNTER: $FUNC_SUCCESS/$FUNC_FAIL grep $str FUNCTIONALITY SUCCESS"
    else
        (( FUNC_FAIL++ ))
        echo "TEST $COUNTER: $FUNC_SUCCESS/$FUNC_FAIL grep $str FUNCTIONALITY FAIL"
    fi
    rm s21_grep_testing.log system_grep_testing.log
}

for opt1 in "${flags[@]}"
do
    for opt2 in "${flags[@]}"
    do
        for opt3 in "${flags[@]}"
        do
            for opt4 in "${flags[@]}"
            do
                if [ $opt1 != $opt2 ] && [ $opt1 != $opt3 ] \
                && [ $opt1 != $opt4 ] && [ $opt2 != $opt3 ] \
                && [ $opt2 != $opt4 ] && [ $opt3 != $opt4 ]
                then
                    for file in "${files[@]}"
                    do
                        options="-f my_patterns.txt -$opt1$opt2 -$opt3$opt4 -e nulla -e viva"
                        testing $commline
                    done
                fi
            done
        done
    done
done

echo "ALL: $COUNTER"
echo "SUCCESS: $FUNC_SUCCESS"
echo "FAIL: $FUNC_FAIL"
