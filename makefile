###################################################
#
# Makefile for SD15-Project
# Creator [Xcode -> Makefile Ver: Feb 14 2007 09:18:41]
# Created: [Mon Sep 29 19:27:02 2014]
#
###################################################

#
# Macros
#

CC = /usr/bin/gcc
CC_OPTIONS = 
LNK_OPTIONS =

EXECUTABLE_NAME = SD15_PRODUCT 


#
# INCLUDE directories for SD15-Project
#

INCLUDE = -I.\
		-ISD15-Project


#
# Build SD15-Project
#

$(EXECUTABLE_NAME) : \
		./entry.o\
		./list.o\
		./main.o\
		./tuple.o\
		./list-private.o\
		./tuple-private.o\
		./entry-private.o
	$(CC) $(LNK_OPTIONS) \
		./entry.o\
		./list.o\
		./main.o\
		./tuple.o\
		./list-private.o\
		./tuple-private.o\
		./entry-private.o\
		-o $(EXECUTABLE_NAME)

clean : 
		rm \
		./entry.o\
		./list.o\
		./main.o\
		./tuple.o\
		./list-private.o\
		./tuple-private.o\
		./entry-private.o\
		$(EXECUTABLE_NAME)

install : $(EXECUTABLE_NAME)
		

#
# Build the parts of SD15-Project
#


# Item # 1 -- entry --
./entry.o : SD15-Project/entry.c
	$(CC) $(CC_OPTIONS) SD15-Project/entry.c -c $(INCLUDE) -o ./entry.o


# Item # 2 -- list --
./list.o : SD15-Project/list.c
	$(CC) $(CC_OPTIONS) SD15-Project/list.c -c $(INCLUDE) -o ./list.o


# Item # 3 -- main --
./main.o : SD15-Project/main.c
	$(CC) $(CC_OPTIONS) SD15-Project/main.c -c $(INCLUDE) -o ./main.o


# Item # 4 -- tuple --
./tuple.o : SD15-Project/tuple.c
	$(CC) $(CC_OPTIONS) SD15-Project/tuple.c -c $(INCLUDE) -o ./tuple.o


# Item # 5 -- list-private --
./list-private.o : SD15-Project/list-private.c
	$(CC) $(CC_OPTIONS) SD15-Project/list-private.c -c $(INCLUDE) -o ./list-private.o


# Item # 6 -- tuple-private --
./tuple-private.o : SD15-Project/tuple-private.c
	$(CC) $(CC_OPTIONS) SD15-Project/tuple-private.c -c $(INCLUDE) -o ./tuple-private.o


# Item # 7 -- entry-private --
./entry-private.o : SD15-Project/entry-private.c
	$(CC) $(CC_OPTIONS) SD15-Project/entry-private.c -c $(INCLUDE) -o ./entry-private.o


##### END RUN ####
