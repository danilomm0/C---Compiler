scanner : driver.o scanner.o
	gcc -Wall *.o -o scanner

driver.o : scanner-driver.c scanner.h
	gcc -Wall -c scanner-driver.c

scanner.o : scanner.c scanner.h
	gcc -Wall -c scanner.c

.PHONY: clean
clean:
	rm -f scanner *.o