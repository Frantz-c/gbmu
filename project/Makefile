NAME	:= gbmu_project

SOURCES	:= $(wildcard sources/*.c)
OBJECTS	:= $(patsubst sources/%.c,objects/%.o,$(SOURCES))
DEPENDS	:= $(patsubst sources/%.c,objects/%.d,$(SOURCES))

all: $(NAME)

$(NAME): objects $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(shell sdl2-config --libs) -lpthread -lm

objects:
	mkdir -p $@

objects/%.o: sources/%.c
	$(CC) -O3 -march=native -c -MMD -Wall -Wextra -I include $(shell sdl2-config --cflags) -o $@ $<

-include $(DEPENDS)

clean:
	rm -rf objects

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
