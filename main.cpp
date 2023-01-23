#include "src/peer.h"
#include <iostream>

int main() {
    Peer p1("aaaa", 12829);
    Peer p2("bbbb", 12830);
    Peer p3("cccc", 12831);
    contact_info info = {"::1", 12829, -1};
    contact_info info2 = {"::1", 12830, -1};
    sleep(1);
    p2.create_connection(&info);
    p3.create_connection(&info);
    p3.create_connection(&info2);
    sleep(1);
    std::cout << "Node 1 connections: " << p1.get_num_connections() << std::endl;
    std::cout << "Node 2 connections: " << p2.get_num_connections() << std::endl;
    std::cout << "Node 3 connections: " << p3.get_num_connections() << std::endl;

    std::cout << "Node 1 num messages: " << p1.get_num_messages() << std::endl;
    p2.send_message(info, "dummy test message pls work\n");
    sleep(1);
    std::cout << "Node 1 num messages: " << p1.get_num_messages() << std::endl;
    std::cout << "\nMessage: " << p1.get_message().body << std::endl;
}