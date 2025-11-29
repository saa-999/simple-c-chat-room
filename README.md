# TCP Socket Chat Room in C

A robust, terminal-based chat application implemented in C using the Berkeley Sockets API. This project demonstrates core networking concepts including TCP connections, I/O multiplexing with select(), and multithreading with pthread.

## Features

* TCP Protocol: Reliable communication between server and clients.
* Multi-Client Support: The server handles multiple concurrent connections using select() (non-blocking I/O multiplexing).
* Real-time Messaging: Clients use separate threads for sending and receiving messages simultaneously.
* IPv4 & IPv6 Support: Designed to work with both address families.
* Nickname Support: Users can set a custom nickname upon joining.

## Prerequisites

To build and run this project, you need a POSIX-compliant operating system (Linux, macOS, or WSL on Windows) with the following tools:

* GCC Compiler (or any standard C compiler)
* Make (Optional, but recommended)

## Installation & Compilation

Clone the repository and compile the source files using GCC.

### 1. Compile the Server
gcc server.c -o server

### 2. Compile the Client
Note: The client requires the pthread library.
gcc client.c -o client -pthread

## Usage

You will need to run the server in one terminal window and clients in separate terminal windows.

### Step 1: Start the Server
Run the server by specifying a port number (e.g., 8080).

./server 8080

Output: Server is running on socket 3...

### Step 2: Connect a Client
Run the client by specifying the server hostname (or IP) and the port.

# Connect to localhost
./client 127.0.0.1 8080

### Step 3: Chatting
1. Enter your nickname when prompted.
2. Start typing messages. Messages will be broadcast to all other connected clients.

## Technical Overview (For Learners)

This project serves as an educational example of network programming in C:

* Server (server.c): Uses the select() system call to monitor multiple file descriptors (sockets) simultaneously. This allows the server to handle new connections and incoming messages without spawning a new thread/process for every client.
* Client (client.c): Uses pthread_create() to spawn a separate thread for receiving messages. This ensures the user can type (stdin) and receive messages (recv) at the same time without blocking.

## License

This project is for educational purposes. Feel free to use and modify it.
