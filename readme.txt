Nathan Greenlaw
CS 372
Project 2

compile ftserver.c:
	gcc -o ftserver ftserver.c
	example usage on flip1: ftserver 45678 

run ftclient.py through python:
	python ftclient.py
	example usage on flip2: python ftclient.py ftclient flip1.engr.oregonstate.edu 45678 -g fileLarge.txt 45679

First start the server than start the clien.
The server will stay open until a sigint is given.
The client closes out after every command finishes.

The program will not work with large text files as I was not able to allocate enough memory for the file causing a segfault.
