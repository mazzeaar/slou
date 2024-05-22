#!/bin/bash

# DISCLAIMER
# i dont really know bash, so i implemented the basic idea and let gpt4o
# guide me to the solution that i wanted. it was annoying, hard and the result is probably implemented terribly,
# but it works and thats all i needed :)

# formatted as "DEPTH FEN EXPECTED"
tests=(
    "6 8/5bk1/8/2Pp4/8/1K6/8/8 w - d6 0 1 824064"
    "6 8/8/1k6/8/2pP4/8/5BK1/8 b - d3 0 1 824064"
    "6 8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1 1440467"
    "6 8/5k2/8/2Pp4/2B5/1K6/8/8 w - d6 0 1 1440467"
    "6 5k2/8/8/8/8/8/8/4K2R w K - 0 1 661072"
    "6 4k2r/8/8/8/8/8/8/5K2 b k - 0 1 661072"
    "6 3k4/8/8/8/8/8/8/R3K3 w Q - 0 1 803711"
    "6 r3k3/8/8/8/8/8/8/3K4 b q - 0 1 803711"
    "4 r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1 1274206"
    "4 r3k2r/7b/8/8/8/8/1B4BQ/R3K2R b KQkq - 0 1 1274206"
    "4 r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1 1720476"
    "4 r3k2r/8/5Q2/8/8/3q4/8/R3K2R w KQkq - 0 1 1720476"
    "6 2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1 3821001"
    "6 3K4/8/8/8/8/8/4p3/2k2R2 b - - 0 1 3821001"
    "5 8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1 1004658"
    "5 5K2/8/1Q6/2N5/8/1p2k3/8/8 w - - 0 1 1004658"
    "6 4k3/1P6/8/8/8/8/K7/8 w - - 0 1 217342"
    "6 8/k7/8/8/8/8/1p6/4K3 b - - 0 1 217342"
    "6 8/P1k5/K7/8/8/8/8/8 w - - 0 1 92683"
    "6 8/8/8/8/8/k7/p1K5/8 b - - 0 1 92683"
    "6 K1k5/8/P7/8/8/8/8/8 w - - 0 1 2217"
    "6 8/8/8/8/8/p7/8/k1K5 b - - 0 1 2217"
    "7 8/k1P5/8/1K6/8/8/8/8 w - - 0 1 567584"
    "7 8/8/8/8/1k6/8/K1p5/8 b - - 0 1 567584"
    "4 8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1 23527"
    "4 8/5k2/8/5N2/5Q2/2K5/8/8 w - - 0 1 23527"
    "6 8/8/8/8/8/8/6k1/4K2R w K -  185867" 
    "6 8/8/8/8/8/8/1k6/R3K3 w Q -  413018" 
    "6 4k2r/6K1/8/8/8/8/8/8 w k -  179869" 
    "6 r3k3/1K6/8/8/8/8/8/8 w q -  367724" 
    "6 4k3/8/8/8/8/8/8/4K2R b K -  899442" 
    "6 4k3/8/8/8/8/8/8/R3K3 b Q -  1001523" 
    "6 4k2r/8/8/8/8/8/8/4K3 b k -  764643" 
    "6 r3k3/8/8/8/8/8/8/4K3 b q -  846648" 
    "6 4k3/8/8/8/8/8/8/R3K2R b KQ -  3517770" 
    "6 r3k2r/8/8/8/8/8/8/4K3 b kq -  2788982" 
    "6 8/8/8/8/8/8/6k1/4K2R b K -  179869" 
    "6 8/8/8/8/8/8/1k6/R3K3 b Q -  367724" 
    "6 4k2r/6K1/8/8/8/8/8/8 b k -  185867" 
    "6 r3k3/1K6/8/8/8/8/8/8 b q -  413018" 
    "6 8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - -  8107539" 
    "6 8/1k6/8/5N2/8/4n3/8/2K5 w - -  2594412" 
    "6 K7/8/2n5/1n6/8/8/8/k6N w - -  588695" 
    "6 k7/8/2N5/1N6/8/8/8/K6n w - -  688780" 
    "6 8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - -  8503277" 
    "6 8/1k6/8/5N2/8/4n3/8/2K5 b - -  3147566" 
    "6 8/8/3K4/3Nn3/3nN3/4k3/8/8 b - -  4405103" 
    "6 K7/8/2n5/1n6/8/8/8/k6N b - -  688780" 
    "6 k7/8/2N5/1N6/8/8/8/K6n b - -  588695" 
    "6 k7/B7/1B6/1B6/8/8/8/K6b w - -  7881673" 
    "6 K7/b7/1b6/1b6/8/8/8/k6B w - -  7382896" 
    "6 B6b/8/8/8/2K5/5k2/8/b6B b - -  9250746" 
    "6 k7/B7/1B6/1B6/8/8/8/K6b b - -  7382896" 
    "6 K7/b7/1b6/1b6/8/8/8/k6B b - -  7881673" 
    "6 6kq/8/8/8/8/8/8/7K w - -  391507" 
    "6 6KQ/8/8/8/8/8/8/7k b - -  391507" 
    "6 K7/8/8/3Q4/4q3/8/8/7k w - -  3370175" 
    "6 6qk/8/8/8/8/8/8/7K b - -  419369" 
    "6 6KQ/8/8/8/8/8/8/7k b - -  391507" 
    "6 K7/8/8/3Q4/4q3/8/8/7k b - -  3370175" 
    "6 8/8/8/8/8/K7/P7/k7 w - -  6249" 
    "6 8/8/8/8/8/7K/7P/7k w - -  6249" 
    "6 K7/p7/k7/8/8/8/8/8 w - -  2343" 
    "6 7K/7p/7k/8/8/8/8/8 w - -  2343" 
    "6 8/2k1p3/3pP3/3P2K1/8/8/8/8 w - -  34834" 
    "6 8/8/8/8/8/K7/P7/k7 b - -  2343" 
    "6 8/8/8/8/8/7K/7P/7k b - -  2343" 
    "6 K7/p7/k7/8/8/8/8/8 b - -  6249" 
    "6 7K/7p/7k/8/8/8/8/8 b - -  6249" 
    "6 8/2k1p3/3pP3/3P2K1/8/8/8/8 b - -  34822" 
    "6 8/8/8/8/8/4k3/4P3/4K3 w - -  11848" 
    "6 4k3/4p3/4K3/8/8/8/8/8 b - -  11848" 
    "6 8/8/7k/7p/7P/7K/8/8 w - -  10724" 
    "6 8/8/k7/p7/P7/K7/8/8 w - -  10724" 
    "6 8/8/3k4/3p4/3P4/3K4/8/8 w - -  53138" 
    "6 8/3k4/3p4/8/3P4/3K4/8/8 w - -  157093" 
    "6 8/8/3k4/3p4/8/3P4/3K4/8 w - -  158065" 
    "6 k7/8/3p4/8/3P4/8/8/7K w - -  20960" 
    "6 8/8/7k/7p/7P/7K/8/8 b - -  10724" 
    "6 8/8/k7/p7/P7/K7/8/8 b - -  10724" 
    "6 8/8/3k4/3p4/3P4/3K4/8/8 b - -  53138" 
    "6 8/3k4/3p4/8/3P4/3K4/8/8 b - -  158065" 
    "6 8/8/3k4/3p4/8/3P4/3K4/8 b - -  157093" 
    "6 k7/8/3p4/8/3P4/8/8/7K b - -  21104" 
    "6 7k/3p4/8/8/3P4/8/8/K7 w - -  32191" 
    "6 7k/8/8/3p4/8/8/3P4/K7 w - -  30980" 
    "6 k7/8/8/7p/6P1/8/8/K7 w - -  41874" 
    "6 k7/8/7p/8/8/6P1/8/K7 w - -  29679" 
    "6 k7/8/8/6p1/7P/8/8/K7 w - -  41874" 
    "6 k7/8/6p1/8/8/7P/8/K7 w - -  29679" 
    "6 k7/8/8/3p4/4p3/8/8/7K w - -  22886" 
    "6 k7/8/3p4/8/8/4P3/8/7K w - -  28662" 
    "6 7k/3p4/8/8/3P4/8/8/K7 b - -  32167" 
    "6 7k/8/8/3p4/8/8/3P4/K7 b - -  30749" 
    "6 k7/8/8/7p/6P1/8/8/K7 b - -  41874" 
    "6 k7/8/7p/8/8/6P1/8/K7 b - -  29679" 
    "6 k7/8/8/6p1/7P/8/8/K7 b - -  41874" 
    "6 k7/8/6p1/8/8/7P/8/K7 b - -  29679" 
    "6 k7/8/8/3p4/4p3/8/8/7K b - -  22579" 
    "6 k7/8/3p4/8/8/4P3/8/7K b - -  28662" 
    "6 7k/8/8/p7/1P6/8/8/7K w - -  41874" 
    "6 7k/8/p7/8/8/1P6/8/7K w - -  29679" 
    "6 7k/8/8/1p6/P7/8/8/7K w - -  41874" 
    "6 7k/8/1p6/8/8/P7/8/7K w - -  29679" 
    "6 k7/7p/8/8/8/8/6P1/K7 w - -  55338" 
    "6 k7/6p1/8/8/8/8/7P/K7 w - -  55338" 
    "6 3k4/3pp3/8/8/8/8/3PP3/3K4 w - -  199002" 
    "6 7k/8/8/p7/1P6/8/8/7K b - -  41874" 
    "6 7k/8/p7/8/8/1P6/8/7K b - -  29679" 
    "6 7k/8/8/1p6/P7/8/8/7K b - -  41874" 
    "6 7k/8/1p6/8/8/P7/8/7K b - -  29679" 
    "6 k7/7p/8/8/8/8/6P1/K7 b - -  55338" 
    "6 k7/6p1/8/8/8/8/7P/K7 b - -  55338" 
    "6 3k4/3pp3/8/8/8/8/3PP3/3K4 b - -  199002" 
    "6 8/Pk6/8/8/8/8/6Kp/8 w - -  1030499" 
    "6 8/Pk6/8/8/8/8/6Kp/8 b - -  1030499" 
    "6 8/8/1k6/8/2pP4/8/5BK1/8 b - d3  824064" 
    "6 8/8/1k6/2b5/2pP4/8/5K2/8 b - d3  1440467" 
    "6 8/5k2/8/2Pp4/2B5/1K6/8/8 w - d6  1440467" 
    "6 5k2/8/8/8/8/8/8/4K2R w K -  661072" 
    "6 4k2r/8/8/8/8/8/8/5K2 b k -  661072" 
    "6 3k4/8/8/8/8/8/8/R3K3 w Q -  803711" 
    "6 r3k3/8/8/8/8/8/8/3K4 b q -  803711" 
    "4 r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq -  1274206" 
    "4 r3k2r/7b/8/8/8/8/1B4BQ/R3K2R b KQkq -  1274206" 
    "4 r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq -  1720476" 
    "4 r3k2r/8/5Q2/8/8/3q4/8/R3K2R w KQkq -  1720476" 
    "6 2K2r2/4P3/8/8/8/8/8/3k4 w - -  3821001" 
    "6 3K4/8/8/8/8/8/4p3/2k2R2 b - -  3821001" 
    "5 8/8/1P2K3/8/2n5/1q6/8/5k2 b - -  1004658" 
    "5 5K2/8/1Q6/2N5/8/1p2k3/8/8 w - -  1004658" 
    "6 4k3/1P6/8/8/8/8/K7/8 w - -  217342" 
    "6 8/k7/8/8/8/8/1p6/4K3 b - -  217342" 
    "6 8/P1k5/K7/8/8/8/8/8 w - -  92683" 
    "6 8/8/8/8/8/k7/p1K5/8 b - -  92683" 
    "6 K1k5/8/P7/8/8/8/8/8 w - -  2217" 
    "6 8/8/8/8/8/p7/8/k1K5 b - -  2217" 
    "7 8/k1P5/8/1K6/8/8/8/8 w - -  567584" 
    "7 8/8/8/8/1k6/8/K1p5/8 b - -  567584" 
    "4 8/8/2k5/5q2/5n2/8/5K2/8 b - -  23527" 
    "4 8/5k2/8/5N2/5Q2/2K5/8/8 w - -  23527" 
    "3 rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq -  53392" 
    "5 rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -  89941194" 
    "5 1k6/1b6/8/8/7R/8/8/4K2R b K -  1063513" 
    "6 3k4/3p4/8/K1P4r/8/8/8/8 b - -  1134888" 
    "6 8/8/4k3/8/2p5/8/B2P2K1/8 w - -  1015133" 
    "1 n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1 24"
    "2 n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1 496"
    "3 n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1 9483"
    "4 n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1 182838"
    "5 n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1 3605103"
    "6 n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1 71179139"
)

NAME="slou"
ENGINE="$(dirname "$0")/bin/$NAME"
COMMAND="-perft"

FORMAT_PRINT="%-10s %-15s %-15s %s"
FORMAT_RESULTS="%-10s %-10s"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'    # reset
PROGRESS_BLOCK='.'

failed_tests=()
total_tests=${#tests[@]}

passed_count=0  # #n passed tests
failed_count=0  # #n failed tests
test_count=0    # #n tests, for progress bar

total_time=0
total_nodes=0

# way to store the failed test in a formatted way for nicer printing later on
store_failed_test() {
    local depth=$1
    local fen=$2
    local expected=$3
    local output=$4

    local actual_output=$(echo "$output" | awk '{print $NF}') # extract the output behind the whitespace
    printf -v formatted "$FORMAT_PRINT" "$depth" "$expected" "$actual_output" "$fen"
    failed_tests+=("$formatted")
}

echo "--------------------------------------"
echo "Running perft tests for $NAME:"
echo "--------------------------------------"
for test in "${tests[@]}"; do
    # extract the parameters from the testcase
    depth=$(echo $test | awk '{print $1}')                                  # first whitespace split
    expected=$(echo $test | awk '{print $NF}')                              # middle part
    fen=$(echo $test | awk '{$1=""; $NF=""; print $0}' | xargs)             # last whitespace split

    # run the test and measure the execution time in ns
    start=$(gdate +%s%N)
    output=$($ENGINE $COMMAND "$depth" "$fen" "$expected" 2>&1)             # run the testcase
    end=$(gdate +%s%N)
    total_time=$(echo "$total_time + $(echo "$end - $start" | bc)" | bc)    # accumulate the duration to the total

    # extract the amount of nodes that was printed
    nodes_line=$(echo "$output" | grep -E "passed:|failed:")
    # here we convert the nodes to a numerical value, idk why but thats the only way i got it to work
    nodes=$((10#$(echo "$(echo "$nodes_line" | awk '{print $2}')" | tr -cd '[:digit:]')))
    total_nodes=$(echo "$total_nodes + $nodes" | bc)                        

    if echo "$output" | grep "passed: " >/dev/null; then
        # print a green symbol if the test passed
        passed_count=$((passed_count + 1))
        echo -ne "${GREEN}$PROGRESS_BLOCK${NC}"
    else
        # store the failed test and print a red symbol
        failed_count=$((failed_count + 1))
        store_failed_test "$depth" "$fen" "$expected" "$output"
        echo -ne "${RED}$PROGRESS_BLOCK${NC}"
    fi

    if (( (test_count + 1) % 38 == 0 ))  ; then
        echo ""
    fi

    test_count=$((test_count + 1))
done

total_time_ms=$(echo "scale=6; $total_time / 1000000" | bc)     # convert from ns to ms for duration
total_time_s=$(echo "scale=9; $total_time / 1000000000" | bc)   # convert from ns to s for nps
nps=$(echo "scale=2; $total_nodes / $total_time_s" | bc)        

# add thousand separator: 1000 -> 1,000
add_separators() { 
    echo "$1" | awk '{printf "%'\''d\n", $0}'
}

total_nodes=$(add_separators "$total_nodes")
nps=$(add_separators "$nps")    

time=$(echo $(echo $total_time_s \* 1000 | bc) | awk -F'.' '{print $1}')    # remove decimal places

# print the test results
echo "--------------------------------------"
printf "$FORMAT_RESULTS\n" "Passed:" "$passed_count/$total_tests"
if (( failed_count > 0 )); then
    printf "$FORMAT_RESULTS\n" "Failed:" "$failed_count/$total_tests"
fi
echo "--------------------------------------"
printf "$FORMAT_RESULTS\n" "Duration:" "$(echo $time)ms"
printf "$FORMAT_RESULTS\n" "Nodes:" "$total_nodes"
printf "$FORMAT_RESULTS\n" "NPS:" "$nps"
echo "--------------------------------------"

# only print this if we failed something
if (( failed_count > 0 )); then
    echo "Failed Tests:"
    printf "$FORMAT_PRINT\n" "Depth" "Expected" "Output" "FEN"
    echo "--------------------------------------"
    for failed_test in "${failed_tests[@]}"; do
        echo "$failed_test"
    done
    echo "--------------------------------------"
fi

exit

# those are kinda slow, so i disabled them for now
#$ENGINE $COMMAND "6" "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - " "179862938" 
#$ENGINE $COMMAND "6" "r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - " "198328929" 
#$ENGINE $COMMAND "6" "r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - " "185959088" 
#$ENGINE $COMMAND "6" "r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - " "190755813" 
#$ENGINE $COMMAND "6" "1r2k2r/8/8/8/8/8/8/R3K2R b KQk - " "195629489" 
#$ENGINE $COMMAND "6" "2r1k2r/8/8/8/8/8/8/R3K2R b KQk - " "184411439" 
#$ENGINE $COMMAND "6" "r3k1r1/8/8/8/8/8/8/R3K2R b KQq - " "189224276" 
#$ENGINE $COMMAND "6" "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - " "179862938" 
#$ENGINE $COMMAND "6" "r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - " "195629489" 
#$ENGINE $COMMAND "6" "r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - " "184411439" 
#$ENGINE $COMMAND "6" "r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - " "189224276" 
#$ENGINE $COMMAND "6" "1r2k2r/8/8/8/8/8/8/R3K2R w KQk - " "198328929" 
#$ENGINE $COMMAND "6" "2r1k2r/8/8/8/8/8/8/R3K2R w KQk - " "185959088" 
#$ENGINE $COMMAND "6" "r3k1r1/8/8/8/8/8/8/R3K2R w KQq - " "190755813" 
#$ENGINE $COMMAND "6" "8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - "  "19870403" 

#$ENGINE $COMMAND "6" "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - " "119060324" 
#$ENGINE $COMMAND "5" "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - " "164075551" 
#$ENGINE $COMMAND "5" "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - " "193690690" 
#$ENGINE $COMMAND "6" "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - " "706045033" 
#$ENGINE $COMMAND "6" "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - " "11030083" 
#$ENGINE $COMMAND "7" "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - " "178633661" 
#$ENGINE $COMMAND "5" "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - " "15833292" 
#$ENGINE $COMMAND "6" "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - " "37665329" 
#$ENGINE $COMMAND "6" "8/PPPk4/8/8/8/8/4Kppp/8 b - - " "28859283" 
#$ENGINE $COMMAND "6" "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - " "71179139" 
#$ENGINE $COMMAND "6" "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - " "37665329" 
#$ENGINE $COMMAND "6" "8/PPPk4/8/8/8/8/4Kppp/8 w - - " "28859283" 
#$ENGINE $COMMAND "6" "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - " "71179139" 
#$ENGINE $COMMAND "6" "B6b/8/8/8/2K5/4k3/8/b6B w - - " "22823890" 
#$ENGINE $COMMAND "6" "8/8/1B6/7b/7k/8/2B1b3/7K w - - " "28861171" 
#$ENGINE $COMMAND "6" "8/8/1B6/7b/7k/8/2B1b3/7K b - - " "29027891" 
#$ENGINE $COMMAND "6" "7k/RR6/8/8/8/8/rr6/7K w - - " "44956585" 
#$ENGINE $COMMAND "6" "R6r/8/8/2K5/5k2/8/8/r6R w - - " "525169084" 
#$ENGINE $COMMAND "6" "7k/RR6/8/8/8/8/rr6/7K b - - " "44956585" 
#$ENGINE $COMMAND "6" "R6r/8/8/2K5/5k2/8/8/r6R b - - " "524966748" 