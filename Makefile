CFLAGS= -Wall -std=gnu99
Targets: CircuitRouter-AdvanceShell CircuitRouter-Client
CircuitRouter-AdvanceShell: CircuitRouter-AdvanceShell.o lib/commandlinereader.o linkedlist.o
	gcc lib/commandlinereader.o CircuitRouter-AdvanceShell.o linkedlist.o -lm -o CircuitRouter-AdvanceShell

lib/commandlinereader.o: lib/commandlinereader.c lib/commandlinereader.h
	gcc $(CFLAGS) -c lib/commandlinereader.c -o lib/commandlinereader.o

CircuitRouter-AdvanceShell.o: CircuitRouter-AdvanceShell.c CircuitRouter-AdvanceShell.h
	gcc $(CFLAGS) -c CircuitRouter-AdvanceShell.c -o CircuitRouter-AdvanceShell.o

linkedlist.o: linkedlist.c linkedlist.h
	gcc $(CFLAGS) -c linkedlist.c -o linkedlist.o

CircuitRouter-Client: CircuitRouter-Client.o 
	gcc CircuitRouter-Client.o  -lm -o CircuitRouter-Client

CircuitRouter-Client.o: CircuitRouter-Client.c
	gcc $(CFLAGS) -c CircuitRouter-Client.c -o CircuitRouter-Client.o


