Below is a summary of each item in the Lab_2_Zipped_Files directory:

examples (directory):
	- This directory contains the output logs of a converged state 4_router.conf network.function

4_router.conf:
	- This .conf file can be used as in input to the network emulator and is a basic 4 router topology configuration

endian.c:
	- This is a file so you can more easily convert provided packets from network to host byte order and vice versa

golden_router:
	- Compiled binary of a working router
	- Your code should perform similar to this golden router

Makefile:
	- file used to compile code, use make router, make unit-test, or make clean accordingly

ne (binary):
	- network emulator binary

router.h:
	- provided source file that students are not allowed to modify. This file contains info about the functions that should be implemented in routingtable.c

routingtable.c:
	- provided skeleton file for the implementation of routingtable.c
	- this skeleton file included a completed PrintRoutes() function

unit-test.c:
	- unit-test source code that the students use to test their routingtable.c code, specifically for the checkpoint submission.
