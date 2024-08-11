CC = gcc -Wvla -Wextra -Werror
CFLAGS =  -D_GNU_SOURCE
DEBUG = -DDEBUG -g -Wall -O0


SRC_DIR = sources
HEAD_DIR = headers
BIN_DIR = bin


# Targets to build the individual executables
build: $(BIN_DIR)/atomo.o $(BIN_DIR)/attivatore.o $(BIN_DIR)/alimentatore.o $(BIN_DIR)/master 

build: $(SRC_DIR)/master.o 
	gcc -o build $(SRC_DIR)/master.o

atomo.o: $(SRC_DIR)/atomo.c $(HEAD_DIR)/atomo.h
	gcc -c $(SRC_DIR)/atomo.c

attivatore.o: $(SRC_DIR)/attivatore.c $(HEAD_DIR)/attivatore.h
	gcc -c $(SRC_DIR)/attivatore.c

master.o: $(SRC_DIR)/master.c $(HEAD_DIR)/master.h
	gcc -c $(SRC_DIR)/master.c

alimentatore.o: $(SRC_DIR)/alimentatore.c $(HEAD_DIR)/alimentatore.h
	gcc -c $(SRC_DIR)/alimentatore.c
	
# Rules to compile and link each program
$(BIN_DIR)/atomo.o: $(SRC_DIR)/atomo.c $(HEAD_DIR)/atomo.h
	$(CC) $(CFLAGS) $(DEBUG) $(SRC_DIR)/atomo.c -o $(BIN_DIR)/atomo

$(BIN_DIR)/attivatore.o: $(SRC_DIR)/attivatore.c $(HEAD_DIR)/attivatore.h
	$(CC) $(CFLAGS) $(DEBUG) $(SRC_DIR)/attivatore.c -o $(BIN_DIR)/attivatore

$(BIN_DIR)/alimentatore.o: $(SRC_DIR)/alimentatore.c $(HEAD_DIR)/alimentatore.h
	$(CC) $(CFLAGS) $(DEBUG) $(SRC_DIR)/alimentatore.c -o $(BIN_DIR)/alimentatore

$(BIN_DIR)/master: $(SRC_DIR)/master.c $(HEAD_DIR)/master.h
	$(CC) $(CFLAGS) $(DEBUG) $(SRC_DIR)/master.c -o $(BIN_DIR)/master

# Clean up
clean:
	rm -f $(BIN_DIR)/* build *.o $(SRC_DIR)/*.o
