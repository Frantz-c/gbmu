# **************************************************************************** #
#                                                           LE - /             #
#                                                               /              #
#    Makefile                                         .::    .:/ .      .::    #
#                                                  +:+:+   +:    +:  +:+:+     #
#    By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+      #
#                                                  #+#   #+    #+    #+#       #
#    Created: 2019/06/19 08:20:37 by mhouppin     #+#   ##    ##    #+#        #
#    Updated: 2019/06/19 17:28:39 by mhouppin    ###    #+. /#+    ###.fr      #
#                                                          /                   #
#                                                         /                    #
# **************************************************************************** #

NAME	:= gbmu_x

OBJECTS	:= execute.o load_cartridge.o sync_test.o

DEPENDS	:= $(patsubst %.o,%.d,$(OBJECTS))

COMPILE	:= gcc

ifneq ($(shell which gcc-8),)
	COMPILE	:= gcc-8
endif
ifneq ($(shell which gcc-9),)
	COMPILE	:= gcc-9
endif

all: $(NAME)

$(NAME): $(OBJECTS)
	$(COMPILE) -o $@ $^ $(shell sdl2-config --libs)

%.o: %.c
	$(COMPILE) -O3 -g3 -march=native -c -MMD $(shell sdl2-config --cflags) -o $@ $<

-include $(DEPENDS)

clean:
	rm -f $(OBJECTS)
	rm -f $(DEPENDS)

fclean: clean
	rm -f $(NAME)

re: fclean all
