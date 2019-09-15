# **************************************************************************** #
#                                                           LE - /             #
#                                                               /              #
#    Makefile                                         .::    .:/ .      .::    #
#                                                  +:+:+   +:    +:  +:+:+     #
#    By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+      #
#                                                  #+#   #+    #+    #+#       #
#    Created: 2019/08/29 10:27:09 by fcordon      #+#   ##    ##    #+#        #
#    Updated: 2019/09/15 15:11:50 by fcordon     ###    #+. /#+    ###.fr      #
#                                                          /                   #
#                                                         /                    #
# **************************************************************************** #



all:
	make -C create_object
	make -C assemble_object
	make -C router

re:
	make $@ -C create_object
	make $@ -C assemble_object
	make $@ -C router

clean:
	make $@ -C 
	make $@ -C assemble_object
	make $@ -C router

fclean:
	make $@ -C 
	make $@ -C assemble_object
	make $@ -C router
