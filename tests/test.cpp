#include "unit_test_framework.h"
#include "peer.h"

#include<unistd.h>
#include <iostream>
#include <string>

// if we run the tests too quickly sequentially, the ports don't get recycled
int current_port = 12829;

// this only creates the objects and does nothing else
// sanity check
// if this fails, something is very wrong
TEST(create_nodes) {
    Peer p1("aaaa", current_port);
    Peer p2("bbbb", current_port+1);
    Peer p3("cccc", current_port+2);
    current_port += 3;

    // just test that it gets here without error
    ASSERT_TRUE(true);
}

// helper function to create nodes and fully connect them
// receives pointers to the lists to fill, then fills the lists
bool generate_connections_helper(int n,
                                 std::vector<Peer*>* peer_list,
                                 std::vector<contact_info*>* contact_infos) 
{
    // check that both vectors are empty
    if ((peer_list->size() != 0) || (contact_infos->size() != 0))
        return false;
    
    // create the nodes
    for (int i = 0; i < n; i++) {
        Peer* p = new Peer(std::to_string(i), current_port);
        contact_info* info = new contact_info{"::1", (uint16_t) current_port, -1};
        current_port++;
        peer_list->push_back(p);
        contact_infos->push_back(info);
    }

    // connect all the nodes
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            // lower ID node always initiates the connection
            if (i >= j)
                continue;
            (*peer_list)[i]->create_connection(std::to_string(j), *(*contact_infos)[j]);
        }
    }
    return true;
}

// create n nodes and O(n^2) connections
TEST(create_connections) {
    // this is the max for now
    // if we go any higher, the OS will stop allocating files
    int n = 20;

    std::vector<Peer*> peer_list;
    std::vector<contact_info*> contact_infos;
    
    // create nodes and connect them
    int ret = generate_connections_helper(n, &peer_list, &contact_infos);
    ASSERT_TRUE(ret);   // make sure it succeeded

    // check that each node is connected to n-1 other nodes
    for (int i = 0; i < n; i++) {
        int num_connections = peer_list[i]->get_num_connections();
        ASSERT_TRUE(num_connections == n-1);
    }

    // just make sure it got this far
    ASSERT_TRUE(true);
}

// test that each node in an n-node network can broadcast and receive broadcasts
// this also tests that nodes can send and receive
TEST(broadcast) {
    int n = 10;

    std::vector<Peer*> peer_list;
    std::vector<contact_info*> contact_infos;
    
    // create nodes and connect them
    int ret = generate_connections_helper(n, &peer_list, &contact_infos);
    ASSERT_TRUE(ret);   // make sure it succeeded

    // have each node send a broadcast to each of its peers
    for (int i = 0; i < n; i++) {
        peer_list[i]->broadcast(std::to_string(i));
        usleep(100000);
    }

    // check that each node received all broadcasts
    for (int i = 0; i < n; i++) {
        // check that there are n-1 messages received
        int num_messages = peer_list[i]->get_num_messages();
        ASSERT_TRUE(num_messages == n-1);

        // check the content of the messages
        for (int j = 0; j < n; j++) {
            if (i == j)
                continue;
            message m = peer_list[i]->get_message();
            // check that the message body is in ascending order
            // this is slightly risky but it's fine
            ASSERT_TRUE(std::stoi(m.body) == j);
        }
    }

    // just make sure it got this far
    ASSERT_TRUE(true);
}



TEST_MAIN()