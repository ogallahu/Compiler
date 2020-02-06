CC    = gcc -std=gnu99

.PHONY: clean test-scanner

bminor: bminor.o scanner.o parser.o utils/expr.o utils/stmt.o utils/type.o  utils/decl.o  utils/param_list.o utils/scope.o utils/hash_table.o  utils/symbol.o utils/scratch.o
	@echo "Compiling bminor..."
	@$(CC) $^ -o $@  

bminor.o: bminor.c token.h
	@echo "Compiling bminor.o..."
	@$(CC) bminor.c -c -o $@

scanner.o: scanner.c token.h
	@echo "Compiling scanner.o..."
	@$(CC) scanner.c -c -o $@

scanner.c: scanner.flex
	@echo "Running flex, producing scanner.c..."
	@flex -o scanner.c scanner.flex

parser.o: parser.c token.h
	@echo "Compiling parser.o..."
	@$(CC) parser.c -c -o $@

parser.c token.h: parser.bison  utils/scope.o utils/hash_table.o  utils/symbol.o  utils/decl.o utils/stmt.o utils/expr.o utils/type.o  utils/param_list.o utils/scratch.o 
	@echo "Running bison, producing parser.c, parser.h"
	@bison --defines=token.h --output=parser.c -v parser.bison

utils/%.o: utils/%.c
	@echo -n "Compiling "
	@echo -n $@
	@echo "..."
	@$(CC) -c $< -o $@

test-scanner: bminor
	@echo "Running instructor tests..."
	@cd ./tests/instructor/scanner; \
		bash run_all_tests.sh; \
		rm *.out


test-type: bminor
	@echo "Running student tests..."
	@cd ./tests/student/typecheck; \
		bash run_all_tests.sh; \
		rm *.out
	@echo "Running instructor tests..."
	@cd ./tests/instructor/typecheck; \
		bash run_all_tests.sh; \
		rm *.out

test-printer: bminor
	@echo "Running instructor tests..."
	@cd ./tests/student/printer; \
		bash run_all_tests.sh; \
		rm *.out

test-parser: bminor
	@echo "Running instructor tests..."
	@cd ./tests/instructor/parser; \
						bash run_all_tests.sh; \
								rm *.out

	@echo "Running hiddenMade tests..."
	@cd ./tests/hidden/parser; \
						bash run_all_tests.sh; \
								rm *.out
	
	@echo "Running userMade tests..."
	@cd ./tests/student/parser; \
						bash run_all_tests.sh; \
								rm *.out
clean:
	@echo "Cleaning scanner..."
	@rm -f scanner.o scanner.c
	@echo "Cleaning parser..."
	@rm -f parser.c token.h parser.o parser.output
	@echo "Cleaning bminor..."
	@rm -f bminor bminor.o
	@echo "Cleaning utils directory..."
	@rm -f utils/*.o
