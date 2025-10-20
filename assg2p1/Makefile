%.o : %.c scanner.h
	gcc -Wall -g -c $< -o $*.o

compile : scanner.o scanner-driver.o driver.o
	gcc *.o -o compile

.PHONY: clean
clean:
	/bin/rm -f *BAK *~ *.o compile
