# **************************************************************************** #
#                                                           LE - /             #
#                                                               /              #
#    Makefile                                         .::    .:/ .      .::    #
#                                                  +:+:+   +:    +:  +:+:+     #
#    By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+      #
#                                                  #+#   #+    #+    #+#       #
#    Created: 2019/06/19 08:20:37 by mhouppin     #+#   ##    ##    #+#        #
#    Updated: 2019/06/24 11:47:51 by mhouppin    ###    #+. /#+    ###.fr      #
#                                                          /                   #
#                                                         /                    #
# **************************************************************************** #

NAME	:= gbmu_x

OBJECTS	:= execute.o load_cartridge.o sync_test.o debug_tools.o

DEPENDS	:= $(patsubst %.o,%.d,$(OBJECTS))

DEBUG	:= -fsanitize=address -g3

COMPILE	?= gcc

all: $(NAME)

$(NAME): $(OBJECTS)
	$(COMPILE) $(DEBUG) -o $@ $^ $(shell sdl2-config --libs)

%.o: %.c
	$(COMPILE) $(DEBUG) -c -MMD $(shell sdl2-config --cflags) -o $@ $<

-include $(DEPENDS)

clean:
	rm -f $(OBJECTS)
	rm -f $(DEPENDS)

fclean: clean
	rm -f $(NAME)

re: fclean all
