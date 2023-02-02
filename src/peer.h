#ifndef PEER_H
#define PEER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <thread>
#include <deque>
#include <mutex>
#include <string>
#include <iostream>
#include <vector>
#include <map>

#define DEFAULT_PORT 12829

typedef struct contact_info {
    const char* ip;
    uint16_t port;
    int sockfd;
} contact_info;

typedef struct message {
    std::string sender_name;
    std::string body;
} message;

class Peer {

public:
    // constructor
    Peer(std::string id, int port);

    // return the number of active connections
    int get_num_connections();

    // return the number of available messages
    int get_num_messages();

    // get the next message off the queue
    message get_message();

    bool create_connection(std::string peer_id, contact_info info);

    void send_message(std::string peer_id, std::string message);
    void broadcast(std::string message);

private:
    std::string id;
    int portnum;

    // connection list
    std::map<std::string, contact_info> connections;
    std::mutex connections_lock;

    // message queue which all receive threads add to
    std::deque<message> message_queue;
    std::mutex message_queue_lock;

    // TCP server, listens for connections and makes receive threads
    void run_server();

    // an infinite receive loop to run as a separate thread
    void receive_loop(int sockfd, std::string peer_id);

};

#endif