%.o : %.c ast.h scanner.h structs.h 
	gcc -Wall -g -c $< -o $*.o

compile : ast-print.o structs.o scanner.o scanner-driver.o driver.o
	gcc *.o -o compile

.PHONY: clean
clean:
	/bin/rm -f *BAK *~ *.o compile
