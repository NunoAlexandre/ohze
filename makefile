###################################################
#
# Makefile - Grupo sd015 - Projecto 4
#
###################################################


#
# Macros
#


#O nome do executavel
EXECUTABLE_CLIENT = SD15_CLIENT
EXECUTABLE_SERVER = SD15_SERVER

CC = /usr/bin/gcc
CC_OPTIONS = -Wall
LNK_OPTIONS = 


#
# INCLUDE directories for SD15-Product
#

INCLUDE = -I.\
		-ISD15-Project


#
# Build SD15-Product
#

$(EXECUTABLE_CLIENT) : \
		./entry.o\
		./list.o\
		./tuple.o\
		./table-cliente.o\
		./network_cliente.o\
		./client_stub.o\
		./general_utils.o\
		./network_utils.o\
		./message.o\
		./table.o
	$(CC) $(LNK_OPTIONS) \
		./entry.o\
		./list.o\
		./tuple.o\
		./table-cliente.o\
		./network_cliente.o\
		./client_stub.o\
		./general_utils.o\
		./network_utils.o\
		./message.o\
		./table.o\
		-o $(EXECUTABLE_CLIENT)

$(EXECUTABLE_SERVER) : \
		./entry.o\
		./list.o\
		./tuple.o\
		./table-server.o\
		./network_server.o\
		./general_utils.o\
		./network_utils.o\
		./message.o\
		./table.o\
		./table_skel.o
	$(CC) $(LNK_OPTIONS) \
		./entry.o\
		./list.o\
		./tuple.o\
		./table-server.o\
		./network_server.o\
		./general_utils.o\
		./network_utils.o\
		./message.o\
		./table.o\
		./table_skel.o\
		-o $(EXECUTABLE_SERVER)

clean : 
		rm \
		./*.o\
		$(EXECUTABLE_CLIENT) \
		$(EXECUTABLE_SERVER)

install : $(EXECUTABLE_SERVER) $(EXECUTABLE_CLIENT)

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


# Item # 4 -- table-cliente --
./table-cliente.o : SD15-Project/table-cliente.c
	$(CC) $(CC_OPTIONS) SD15-Project/table-cliente.c -c $(INCLUDE) -o ./table-cliente.o


# Item # 5 -- network_cliente --
./network_cliente.o : SD15-Project/network_cliente.c
	$(CC) $(CC_OPTIONS) SD15-Project/network_cliente.c -c $(INCLUDE) -o ./network_cliente.o

# Item # 5 -- client_stub --
./client_stub.o : SD15-Project/client_stub.c
	$(CC) $(CC_OPTIONS) SD15-Project/client_stub.c -c $(INCLUDE) -o ./client_stub.o

# Item # 6 -- table-server --
./table-server.o : SD15-Project/table-server.c
	$(CC) $(CC_OPTIONS) SD15-Project/table-server.c -c $(INCLUDE) -o ./table-server.o


# Item # 7 -- general_utils --
./general_utils.o : SD15-Project/general_utils.c
	$(CC) $(CC_OPTIONS) SD15-Project/general_utils.c -c $(INCLUDE) -o ./general_utils.o


# Item # 8 -- network_server --
./network_server.o : SD15-Project/network_server.c
	$(CC) $(CC_OPTIONS) SD15-Project/network_server.c -c $(INCLUDE) -o ./network_server.o


# Item # 9 -- network_utils --
./network_utils.o : SD15-Project/network_utils.c
	$(CC) $(CC_OPTIONS) SD15-Project/network_utils.c -c $(INCLUDE) -o ./network_utils.o


# Item # 10 -- message --
./message.o : SD15-Project/message.c
	$(CC) $(CC_OPTIONS) SD15-Project/message.c -c $(INCLUDE) -o ./message.o


# Item # 11 -- table --
./table.o : SD15-Project/table.c
	$(CC) $(CC_OPTIONS) SD15-Project/table.c -c $(INCLUDE) -o ./table.o


./table_skel.o : SD15-Project/table_skel.c
	$(CC) $(CC_OPTIONS) SD15-Project/table_skel.c -c $(INCLUDE) -o ./table_skel.o

##### END RUN ####
