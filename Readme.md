Grigore Iulia-Anita, 322CA

# Homework 2 - Client-Server application

## Server

The server functionality of the application is incapsulated in the
`server.cpp` file. Here, the stdin input and the one from the TCP
and UDP servers are handled with `select()`. If the set file descriptor
is the UDP one, get the messages from the UDP clients with the help of
`Server_UDP::handle_message` function and forward them to the tcp clients with
`Server_TCP::share_messages`. If it's a TCP file descriptor, handle the
connection (`Server_TCP::handle_client`). If it's an input received from stdin,
it has to be `exit`, otherwise it's an error.

### `Server_UDP`

The UDP server has a pretty simple functionality: open a socket and then
receive messages from the UDP clients and add them to the `pending_messages`
queue.

### `Server_TCP`

The TCP server basically does everything. Accepts connections and adds them to
the list of `pending_conn`. Reads messages from tcp clients and handles them
with `handle_message` function. Forwards the messages from the UDP clients
(which were previously saved in `pending_messages`) to the clients that
subscribed to specific topics (`share_messages`).

#### `handle_message`

For each type of message, I created functions that handles them.

`handle_hello_message`: Check if the hello message is sent from an existent
and active client, remove it from pending connections and return an error,
because that means that the username is already taken.
If the client is existent, but not active, or not existent, update the
information about it, erase from pending and print a "new client" message.

`handle_subscribe_message`: If the connection is not in pending, return -1
because a hello message is expected. Otherwise, check if that subscription
already exists: -> yes: ignore it
                -> no: add it to subscriptions list

`handle_unsubscribe_message`: Like the `handle_subscribe_message`, return -1
if the connection is not in pending. Otherwise, check if it is in subscriptions
list and erase it. If it's not, just ignore it.

#### `disconnect_client`

First, check if the client actually exists before disconnecting it. Then, check
if the store and forward flag is set
-> no: remove all the information about the client.
-> yes: just make the client inactive (set his file descriptor to -1).

## Client

The client functionality is incapsulated in `client.cpp` file. It uses members
and functions declared and implemented in class `Client_TCP`.

### `Client_TCP`

`start()`: Opens a socket and sends a connection request. Returns the file
descriptor.

`say_hello`: Creates a message containing the code 0 (hello code)and the id
of the client and sends it to the server.

`handle_command`: Reads from input and parses the message. For subscribe and
unsubscribe messages, adds the header, saves/removes the topic for further use
and then sends the new message to the server. If the message is "exit",
return 0.

`handle_server_message`: Reads the messages and concatenates it with the
fragment saved, to avoid sending partial messages. Then, remove the part
that was read from `data_fragment` to avoid duplicated segments.
