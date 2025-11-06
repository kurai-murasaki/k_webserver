#include <stdio.h>

#include "k_srv.h"


int main() {


    struct K_server *server = create_server(
        AF_INET,
        8080,
        SOCK_STREAM,
        0,
        10,
        INADDR_ANY,
        launch
    );

    server->launch(server);
    return 0;
}