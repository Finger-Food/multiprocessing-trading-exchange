echo "----- UNIT TESTS -----"
ut_dir=tests/ut
$ut_dir/map_ut
$ut_dir/queue_ut
$ut_dir/product_book_ut

printf '\n'
echo "----- E2E TESTS -----"
e2e_dir=tests/e2e
e2e_out_dir=$e2e_dir/out
products=$e2e_dir/products.txt

testsPassed=0
testsFailed=0
red='\033[0;31m'
green='\033[0;32m'
nc='\033[0m'

printf "Running test. Test Autotrader: "
testfile=t_autotrader
output="$(diff $e2e_out_dir/$testfile.out <(./pe_exchange $products $e2e_dir/$testfile pe_trader))"
if [ -z "$output" ];
then
	printf "${green}Passed${nc}\n"
	((testsPassed++))
else
	printf "${red}Failed${nc}\n"
	printf "${output}\n"
	((testsFailed++))
fi

printf "Running test. Test Amend: "
testfile=t_amend
output="$(diff $e2e_out_dir/$testfile.out <(./pe_exchange $products $e2e_dir/$testfile))"
if [ -z "$output" ];
then
	printf "${green}Passed${nc}\n"
	((testsPassed++))
else
	printf "${red}Failed${nc}\n"
	printf "${output}\n"
	((testsFailed++))
fi

printf "Running test. Test Cancel: "
testfile=t_cancel
output="$(diff $e2e_out_dir/$testfile.out <(./pe_exchange $products $e2e_dir/$testfile))"
if [ -z "$output" ];
then
	printf "${green}Passed${nc}\n"
	((testsPassed++))
else
	printf "${red}Failed${nc}\n"
	printf "${output}\n"
	((testsFailed++))
fi

testfile=t_basic
printf "Running test. Test Basic: "
output="$(diff $e2e_out_dir/$testfile.out <(./pe_exchange $products $e2e_dir/$testfile"0" $e2e_dir/$testfile"1" ))"
if [ -z "$output" ];
then
	printf "${green}Passed${nc}\n"
	((testsPassed++))
else
	printf "${red}Failed${nc}\n"
	printf "${output}\n"
	((testsFailed++))
fi

printf "Running test. Test Complex: "
testfile=t_complex
output="$(diff $e2e_out_dir/$testfile.out <(./pe_exchange $products $e2e_dir/$testfile"0" $e2e_dir/$testfile"1" ))"
if [ -z "$output" ];
then
	printf "${green}Passed${nc}\n"
	((testsPassed++))
else
	printf "${red}Failed${nc}\n"
	printf "${output}\n"
	((testsFailed++))
fi

printf "E2E tests: ${green}${testsPassed} passed${nc}, ${red}${testsFailed} failed${nc}\n"
