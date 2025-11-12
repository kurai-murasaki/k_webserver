#include <stdio.h>

#include "k_srv.h"


int main(int argc, char *argv[]){


    struct K_server *server = k_create_server(
        AF_INET,
        8080,
        SOCK_STREAM,
        0,
        10,
        INADDR_ANY,
        k_launch
    );

    server->launch(server);
    return 0;
}