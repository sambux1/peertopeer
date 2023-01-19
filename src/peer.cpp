#include "peer.h"

Peer::Peer(std::string id, int port) {
    this->id = id;

    // check that the port number is valid, set default if not
    if ((port > 65535) || (port < 1024)) {
        this->portnum = (uint16_t) DEFAULT_PORT;
    } else {
        this->portnum = (uint16_t) port;
    }

    // start the server to accept incoming connections
    std::thread t(&Peer::run_server, this);
    t.detach();
}

int Peer::get_num_connections() {
    this->connections_lock.lock();
    int num = this->connections.size();
    this->connections_lock.unlock();

    return num;
}

int Peer::create_connection(contact_info info) {
    const char* port = std::to_string(info.port).c_str();

    int sockfd;
    struct addrinfo* server_info;
    int yes = 1;

    // basic information about the server
    // a TCP server with an IPv6 address
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get information about the server to connect to
    int ret;
    if ((ret = getaddrinfo(info.ip, port, &hints, &server_info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return 1;
    }
    
    // iterate through the linked list of candidate server information structs
    struct addrinfo* a;
    for (a = server_info; a != NULL; a = a->ai_next) {
        if ((sockfd = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, a->ai_addr, a->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
        }

        break;
    }

    // check if it failed to connect to any of the candidates
    if (a == NULL) {
        fprintf(stderr, "failed to connect");
        return 0;
    }

    freeaddrinfo(server_info);

    // if it gets here, the connection is established

    // update the sockfd field
    info.sockfd = sockfd;
    // add the new connection to the connection list
    this->connections_lock.lock();
    this->connections.push_back(info);
    this->connections_lock.unlock();

    // create a listener thread
    std::thread receive_thread(&Peer::receive_loop, this, sockfd);
    receive_thread.detach();
    return 1;
}

void Peer::run_server() {
    const char* port = std::to_string(this->portnum).c_str();

    int sockfd;
    struct addrinfo* server_info;
    int yes = 1;

    // basic information about the server
    // a TCP server with an IPv6 address
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // autofill the server information, returns a linked list of candidates
    int ret;
    if ((ret = getaddrinfo(NULL, port, &hints, &server_info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return;
    }

    // iterate through the linked list of candidate server information structs
    struct addrinfo* a;
    for (a = server_info; a != NULL; a = a->ai_next) {
        // create the socket
        if ((sockfd = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) == -1) {
            perror("Socket creation failed");
            continue;
        }
        // allow the port to become available again immediately after program ends
        // mostly helpful for debugging and testing
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt failed");
            exit(1);
        }
        // bind the socket to the address and port
        if (bind(sockfd, a->ai_addr, a->ai_addrlen) == -1) {
            perror("Socket binding failed");
            close(sockfd);
            continue;
        }

        break;
    }

    // check to make sure the server setup was successful
    if (a == NULL) {
        fprintf(stderr, "failed to start server");
        exit(1);
    }

    freeaddrinfo(server_info);

    // start listening for connections with a backlog size of 10
    if (listen(sockfd, 10) == -1) {
        perror("listen");
        exit(1);
    }
    
    // infinite accept loop
    while (true) {
        struct sockaddr_storage client_addr;
        socklen_t sin_size = sizeof(client_addr);

        // accept the new connection
        int newfd;
        newfd = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size);
        if (newfd == -1) {
            perror("accept");
            continue;
        }

        // get the IP address and port from the connection
        struct sockaddr_in* address = (struct sockaddr_in*) ((struct sockaddr*) &client_addr);
        char client_ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &address->sin_addr, client_ip, sizeof(client_ip));
        uint16_t client_port = htons(address->sin_port);

        contact_info info = {client_ip, client_port, newfd};

        // add the new connection to the connection list
        this->connections_lock.lock();
        this->connections.push_back(info);
        this->connections_lock.unlock();

        std::thread receive_thread(&Peer::receive_loop, this, newfd);
        receive_thread.detach();
    }
}

void Peer::receive_loop(int sockfd) {
    // temporarily limit message size to 100 bytes
    char buf[100];
    int count = 0;
    while (true) {
        int numbytes;
        if ((numbytes = recv(sockfd, buf, 99, 0)) <= 0) {
            break;
        }
        buf[numbytes] = '\0';
        std::string s(buf);
        
        this->message_queue_lock.lock();
        this->message_queue.push_back(s);
        this->message_queue_lock.unlock();
    }

    // connection has failed
    // close the socket and terminate the thread
    close(sockfd);
}
