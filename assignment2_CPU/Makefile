###################################################
# SSC-0112 - Organizacao de Computadores Digitais #
# Turma A - 2018/01                               #
# Prof. Paulo Sergio Lopes de Souza               #
#                                                 #
# Makefile para CPU MIPS Multiciclo de 32 bits    #
#                                                 #
# Alunos:                                         #
#     Felipe Scrochio Custodio     - 9442688      #
#     Gabriel Henrique Scalici     - 9292970      #
#     Juliano Fantozzi             - 9791218      #
#     Andre Luis Storino Junior    - 9293668      #
###################################################

# compiler
CC = gcc

# output
BINARY = ./cpu_multi_code
BIN = code.bin

# sources
SRC = ./cpu_multi_code.c

# includes
INCLUDES = .

# compiler flags
# FLAGS = -g -std=c99 -lm -Wall -pedantic
FLAGS = -g -lm -pedantic

# in case ANSI C is necessary
# FLAGS = -g -std=c89 -lm -Wall

all: compile run

compile:
	$(CC) -o $(BINARY) -I $(INCLUDES) $(SRC) $(FLAGS)

run:
	$(BINARY) $(BIN)

debug:
	valgrind --leak-check=full --show-reachable=yes --track-origins=yes $(BINARY)

clean:
	rm *.o
