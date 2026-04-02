NAME := main

SRC := main.c
OBJS :=main.o
CC := gcc
CFLAGS := -Wall -Wextra

RM := rm -f

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(NAME) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

make: all

run: $(NAME)
	./$(NAME)

clean:
	$(RM) $(OBJS)

.PHONY: all make run clean 
