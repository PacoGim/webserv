make

if [[ $# -ge 1 ]]; then
	if [[ ! -f "$1" ]]; then
		echo -e "\e[31mError: '$1' is not a valid file.\e[0m"
		exit 1
	else
		srcs="$1"
	fi
else
	srcs=$(find ./srcs -type f -name "*.cpp")
fi

objects=$(find ./obj -type f -name "*.o" ! -name "main.o")
includes=$(find . -type f -name '*.hpp' -exec dirname {} \; | sort -u | sed 's/^/-I/')

testDirExec=test_dir_exec
mkdir -p $testDirExec

recompile=0

testExecFiles=$(find $testDirExec)

for testExecFile in $testExecFiles; do
	if [[ ./webserv -nt $testExecFile ]]; then
		recompile=1
	fi
done

recompile=0

for src in $srcs; do
	fileName=$(basename "$src")
	testPath="tests/test_$fileName"
	testExec=$(basename "${fileName%.cpp}.test")
	testExecPath="$testDirExec/$testExec"
	
	if [[ -f "$testPath" ]]; then
		if [[ "$src" -nt "$testExecPath" || "$testPath" -nt "$testExecPath" || -x "$testExecPath" || recompile -eq 1 ]]; then
			echo -e "\e[94mTesting $fileName\e[0m"
			g++ -Wall -Wextra -Werror -std=c++98 -g "$includes" "$testPath" BootStrap.cpp "$objects" -o "$testExecPath"
			if [[ $? -ne 0 ]]; then
				echo -e "\e[31mCompilation failed for $fileName\e[0m"
				continue
			fi
			valgrind ./"$testExecPath"
			if [[ $? -eq 0 ]]; then
				echo -e "\e[32mTest Success!\e[0m"
				chmod -x "$testExecPath"
			else
				echo -e "\e[31mTest Failed!\e[0m"
				touch "$testExecPath"
				chmod +x "$testExecPath"
				exit 1
			fi
		fi
	fi
done
