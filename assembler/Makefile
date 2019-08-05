# **************************************************************************** #
#                                                           LE - /             #
#                                                               /              #
#    Makefile                                         .::    .:/ .      .::    #
#                                                  +:+:+   +:    +:  +:+:+     #
#    By: mhouppin <marvin@le-101.fr>                +:+   +:    +:    +:+      #
#                                                  #+#   #+    #+    #+#       #
#    Created: 2019/02/25 16:43:57 by mhouppin     #+#   ##    ##    #+#        #
#    Updated: 2019/08/05 11:50:53 by fcordon     ###    #+. /#+    ###.fr      #
#                                                          /                   #
#                                                         /                    #
# **************************************************************************** #

NAME      = gbasm
SHELL     = bash
OBJ_DIR   = objects
SRC_DIR   = sources
INC_DIR   = includes:vector
SOURCES  := $(wildcard ${SRC_DIR}/*.c)
OBJECTS  := $(patsubst ${SRC_DIR}/%.c,${OBJ_DIR}/%.o,${SOURCES})
DEPENDS  := $(patsubst ${SRC_DIR}/%.c,${OBJ_DIR}/%.d,${SOURCES})
OBJECTS  += vector/vector.o
CFLAGS   ?= -Wextra -Wall -g3
CINC     := -I $(subst :, -I ,${INC_DIR})
CC       ?= gcc-9


.PHONY: all clean fclean re allc
# Chemins par defaut des .h


all: $(OBJ_DIR) $(NAME)


# Creation du repertoire pour les .o
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Creation de l'executable
$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(ARCHIVE)

# Creation des .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(CINC) -c $< -o $@

vector/%.o: vector/%.c
	$(CC) $(CFLAGS) $(CINC) -c -MMD $< -o $@

# inclusion des regles contenues dans les .d
-include $(DEPENDS)


clean:
	/bin/rm -rf $(OBJ_DIR)

fclean: clean
	/bin/rm -f $(NAME)

ffclean: fclean
	/bin/rm -f $(DEPENDS)

re: fclean all

allc: all clean
