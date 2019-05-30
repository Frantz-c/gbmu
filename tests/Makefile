NAME = gbmul
CC = gcc -Wextra -Wall 
FILES = load_cartridge.o\
main.o\
mbc_switch.o

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
