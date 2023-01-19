# The `Peer` Class

A `Peer` object is a node in the peer to peer network.
The interface for interacting with the object is described below.

### Create Peer

`Peer(std::string id, int port)`

Inputs:

- `std::string id` - a unique identifier, particularly useful for local testing with multiple peers
- `int port` - the port to run the peer's server on, legal values are [1024, 65535], default value is 12829 (pass an invalid number for the default)

Example:

`Peer p("peername", 12829);`

### Establish Connections

Once a `Peer` object is created, it automatically starts a TCP server to listen for and accept incoming connections from any peers that initiate a connection.

To initiate a connection, you must supply the target's IP address and port number in the form of a `contact_info` struct, defined below.

```
struct contact_info {
    const char* ip;
    uint16_t port;
    int sockfd;
};
```

- `const char* ip` - the target's IP address as a C string, e.g. `"127.0.0.1"` or `"::1"`
- `uint16_t port` - the target's port number
- `int sockfd` - I'll do something about this, it shouldn't be editable

<br />

Connections can be created one-by-one or in a batch.

To create a single connection:

`int create_connection(contact_info info)`

Returns:
- 1 if connection succeeds
- 0 if connection fails

<br />

To create multiple connections in a batch, you supply a vector of contact info structs:

`int create_batch_connections(std::vector<contact_info> info_list)`

Returns:
- the number of successful connections from the batch

### Sending and Receiving Messages

#### Sending

To send a message, use the `send` method, which takes as input the contact info of the peer and the message to be sent. This method has the same interface for both TCP and UDP messages.

`void send(contact_info peer, std::string message);`

#### Broadcasting
In addition to sending a message directly to one peer, it is also possible to broadcast a message to all connected peers (TCP) and all known peers (UDP).

To broadcast to all TCP connections, simply pass the message as input.

`void broadcast(std::string message);`

Broadcasting to all known UDP connections is handled by the distributed hash table.

#### Receiving
Receiving messages is handled automatically. Incoming messages are added to a queue which can be read from.

The peer maintains a listening thread for each TCP connection and a single listening thread for all UDP connections.

To read the next message from the message queue:

`message get_message();`

The return value is a message struct, which contains both the string sent and additional information about the sender.

*Insert struct info here*