CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = 
EXEC = lms

SRC = main.c syllabus.c qcm.c progression.c contenu.c persistence.c utils.c
OBJ = $(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJ) $(EXEC)

mrproper: clean
	rm -f database.json

.PHONY: all clean mrproper