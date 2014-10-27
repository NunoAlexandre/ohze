//
//  network_client-private.h
//  SD15-Product
//
//  Created by Nuno Alexandre on 25/10/14.
//  Copyright (c) 2014 Nuno Alexandre. All rights reserved.
//

#ifndef SD15_Product_network_client_private_h
#define SD15_Product_network_client_private_h


/* the struct server_t has an ip adress, 
 a port number and a sock file descriptor*/
struct server_t {
    char * ip_address;
    int port;
    int socketfd;
};


#endif
