%.o : %.c ast.h scanner.h structs.h 
	gcc -Wall -Werror -g -c $< -o $*.o

compile : ast-print.o structs.o scanner.o scanner-driver.o driver.o
	gcc -Wall -Werror -g *.o -o compile

.PHONY: clean
clean:
	/bin/rm -f *BAK *~ *.o compile
