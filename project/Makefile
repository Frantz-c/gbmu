# **************************************************************************** #
#                                                           LE - /             #
#                                                               /              #
#    Makefile                                         .::    .:/ .      .::    #
#                                                  +:+:+   +:    +:  +:+:+     #
#    By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+      #
#                                                  #+#   #+    #+    #+#       #
#    Created: 2019/08/09 11:57:29 by mhouppin     #+#   ##    ##    #+#        #
#    Updated: 2019/08/09 12:10:25 by mhouppin    ###    #+. /#+    ###.fr      #
#                                                          /                   #
#                                                         /                    #
# **************************************************************************** #

NAME	:= gbmu_project

SOURCES	:= $(wildcard sources/*.c)
OBJECTS	:= $(patsubst sources/%.c,objects/%.o,$(SOURCES))
DEPENDS	:= $(patsubst sources/%.c,objects/%.d,$(SOURCES))

OPTIMIZE:= -O3 -march=native
ERRORS	:= -Wall -Wextra -Werror
DEBUG	:= -fsanitize=undefined -g3

all: $(NAME)

$(NAME): objects $(OBJECTS)
	$(CC) $(DEBUG) -o $@ $(OBJECTS) $(shell sdl2-config --libs) -lpthread -lm

objects:
	mkdir -p $@

objects/%.o: sources/%.c
	$(CC) $(OPTIMIZE) $(DEBUG) -c -MMD $(ERRORS) -I include $(shell sdl2-config --cflags) -o $@ $<

-include $(DEPENDS)

clean:
	rm -rf objects

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
