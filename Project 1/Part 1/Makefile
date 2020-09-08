#INSTRUCTIONS:
#1.Type "make" into the command line to compile your code
#2.Type "make test-short" to run the binary with "path_short.txt"
#3.Type "make test-very-long" to run the binary with "path_very_long.txt"
#NOTE : Make sure to always run "make" after editing your ".c" file
CC = gcc
CLIENT = "httpclient"
SERVER = "dtunes.ecn.purdue.edu"
PORT = "80"

all : clean client test-short

client : httpclient.*.c
	$(CC) httpclient.*.c -o $(CLIENT) 

#NOTE : This command will write the output to "out1.txt"
test-short :
	./$(CLIENT) $(SERVER) $(PORT) /ece463/lab1/path_short.txt > out1.txt

#NOTE : This command will write the output to "out2.txt"
test-very-long :
	./$(CLIENT) $(SERVER) $(PORT) /ece463/lab1/path_very_long.txt > out2.txt

clean:
	rm -f httpclient httpclient.jhelfman *.o out1.txt vallog testgen
	clear