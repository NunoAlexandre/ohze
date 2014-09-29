###################################################
#
# Makefile - Grupo sd015
#
###################################################

#
# Macros
#

CC = /usr/bin/gcc
CC_OPTIONS = 
LNK_OPTIONS = 

#O nome do executavel
EXECUTABLE_NAME = SD15_PRODUCT 

#
# INCLUDE directories for SD15-Product
#

INCLUDE = -I.\
		-ISD15-Project


#
# Build SD15-Product
#

$(EXECUTABLE_NAME) : \
		./entry.o\
		./list.o\
		./main.o\
		./tuple.o
	$(CC) $(LNK_OPTIONS) \
		./entry.o\
		./list.o\
		./main.o\
		./tuple.o\
		-o $(EXECUTABLE_NAME)

clean : 
		rm \
		./*.o\
		$(EXECUTABLE_NAME)

install : $(EXECUTABLE_NAME)

#
# Build the parts of SD15-Product
#

# -- main --
./main.o : SD15-Project/main.c
	$(CC) $(CC_OPTIONS) SD15-Project/main.c -c $(INCLUDE) -o ./main.o

#  -- list --
./list.o : SD15-Project/list.c
	$(CC) $(CC_OPTIONS) SD15-Project/list.c -c $(INCLUDE) -o ./list.o


# -- entry --
./entry.o : SD15-Project/entry.c
	$(CC) $(CC_OPTIONS) SD15-Project/entry.c -c $(INCLUDE) -o ./entry.o


# -- tuple --
./tuple.o : SD15-Project/tuple.c
	$(CC) $(CC_OPTIONS) SD15-Project/tuple.c -c $(INCLUDE) -o ./tuple.o


##### END OF MAKEFILE SD015 ####
