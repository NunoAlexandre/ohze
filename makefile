###################################################
#
# Makefile - Grupo sd015 - Projecto 2
#
###################################################

#
# Macros
#

CC = /usr/bin/gcc
CC_OPTIONS = 
LNK_OPTIONS = 


#
# INCLUDE directories for SD15-Product
#

INCLUDE = -I.\
		-ISD15-Project



#O nome do executavel
EXECUTABLE_MESSAGE = SD15_MESSAGE
EXECUTABLE_TABLE = SD15_TABLE
#
# Build SD15-Product
#

$(EXECUTABLE_MESSAGE) : \
		./entry.o\
		./list.o\
		./tuple.o\
		./test_message.o\
		./message.o\
		./table.o
	$(CC) $(LNK_OPTIONS) \
		./entry.o\
		./list.o\
		./tuple.o\
		./test_message.o\
		./message.o\
		./table.o\
		-o $(EXECUTABLE_MESSAGE)

$(EXECUTABLE_TABLE) : \
		./entry.o\
		./list.o\
		./tuple.o\
		./test_table.o\
		./message.o\
		./table.o
	$(CC) $(LNK_OPTIONS) \
		./entry.o\
		./list.o\
		./tuple.o\
		./test_table.o\
		./message.o\
		./table.o\
		-o $(EXECUTABLE_TABLE)


clean : 
		rm \
		./*.o\
		$(EXECUTABLE_TABLE) \
		$(EXECUTABLE_MESSAGE)

install : $(EXECUTABLE_TABLE) $(EXECUTABLE_MESSAGE)

#
# Build the parts of SD15-Product
#


# Item # 1 -- entry --
./entry.o : SD15-Project/entry.c
	$(CC) $(CC_OPTIONS) SD15-Project/entry.c -c $(INCLUDE) -o ./entry.o


# Item # 2 -- list --
./list.o : SD15-Project/list.c
	$(CC) $(CC_OPTIONS) SD15-Project/list.c -c $(INCLUDE) -o ./list.o


# Item # 3 -- tuple --
./tuple.o : SD15-Project/tuple.c
	$(CC) $(CC_OPTIONS) SD15-Project/tuple.c -c $(INCLUDE) -o ./tuple.o


# Item # 4 -- test_message --
./test_message.o : testes_projeto2-GRUPO015/test_message.c
	$(CC) $(CC_OPTIONS) testes_projeto2-GRUPO015/test_message.c -c $(INCLUDE) -o ./test_message.o


# Item # 5 -- test_table --
./test_table.o : testes_projeto2-GRUPO015/test_table.c
	$(CC) $(CC_OPTIONS) testes_projeto2-GRUPO015/test_table.c -c $(INCLUDE) -o ./test_table.o


# Item # 6 -- message --
./message.o : SD15-Project/message.c
	$(CC) $(CC_OPTIONS) SD15-Project/message.c -c $(INCLUDE) -o ./message.o


# Item # 7 -- table --
./table.o : SD15-Project/table.c
	$(CC) $(CC_OPTIONS) SD15-Project/table.c -c $(INCLUDE) -o ./table.o


##### END MAKEFILE GRUPO SD015 ####
