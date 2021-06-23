CC = gcc  
FINAL_EXEC = qSim
MAIN_FILE = main.c
SRC = src
BIN = bin
OBJ = obj
OUTPUT = output
DATA_FILE = data.txt
MATH = m
LIB = lib

.PHONY: qSim
qSim: clean
	@cd src;	$(CC) $(MAIN_FILE) -o ../$(BIN)/$(FINAL_EXEC) -l$(MATH)


.PHONY: execute
execute: qSim
	@./$(BIN)/$(FINAL_EXEC)


.PHONY: clean
clean:
	@rm ./$(OBJ)/*.o -f
	@rm ./$(OUTPUT)/*.* -f
	@rm ./$(BIN)/*.* -f 
	@rm ./$(LIB)/*.a -f
	@rm $(DATA_FILE) -f