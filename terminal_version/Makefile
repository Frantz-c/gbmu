NAME = gbmul
CC = gcc -Wextra -Wall -O3
FILES = load_cartridge.o\
main.o\
execute.o

all: $(NAME)

$(NAME): $(FILES)
	@$(CC) -o $@ $^ 

.c.o:
	@$(CC) -c $<

clean:
	@/bin/rm -f $(FILES)

fclean: clean
	@/bin/rm -f $(NAME)

re: fclean all

allc: all clean
