#INSTRUCTIONS:
#1.Type "make" into the command line to compile your code
#2.Type "make test-short" to run the binary with "path_short.txt"
#3.Type "make test-very-long" to run the binary with "path_very_long.txt"
#NOTE : Make sure to always run "make" after editing your ".c" file
CC = gcc
CLIENT = "httpclient"
SERVER = "dtunes.ecn.purdue.edu"
PORT = "2045"

all : client server fork multi

client : httpclient.*.c
	$(CC) httpclient.*.c -o $(CLIENT) 

server : httpserver.*.c
	$(CC) httpserver.*.c -o httpserver

fork: httpserver_fork.*.c
	$(CC) httpserver_fork.*.c -o httpserverF

multi: test.c multi_service_server.*.c
	#$(CC) test.c -o multi_service_server
	$(CC) multi_service_server.*.c -o multi_service_server

runs: httpserver
	./httpserver $(PORT)

runf: httpserverF
	./httpserverF $(PORT)

runm: multi_service_server
	./multi_service_server $(PORT) "2046"

#NOTE : This command will write the output to "out1.txt"
test-short :
	./$(CLIENT) $(SERVER) $(PORT) /ece463/lab1/path_short.txt > out1.txt

#NOTE : This command will write the output to "out2.txt"
test-very-long :
	./$(CLIENT) $(SERVER) $(PORT) /ece463/lab1/path_very_long.txt > out2.txt

clean:
	rm -f httpclient httpserver httpserverF *.o out1.txt out2.txt vallog testgen
	clear