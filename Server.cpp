#include "Server.h"

Server::Server() {
    // Create a listening socket and set it up to accept incoming client connections
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Error creating socket.\n";
        exit(1);
    }

    int port = 8080; // Change this to your desired port number

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Error binding socket to port.\n";
        exit(1);
    }

    if (listen(serverSocket, 5) < 0) {
        cerr << "Error listening for incoming connections.\n";
        exit(1);
    }
}

Server::~Server() {
    close(serverSocket);

    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void Server::start() {
    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            cerr << "Error accepting client connection.\n";
            continue;
        }

        thread clientThread(&Server::handleClient, this, clientSocket);
        clientThreads.push_back(move(clientThread));
    }
}

void Server::handleClient(int clientSocket) {
    while(true){
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead < 0) {
            cerr << "Error reading from client socket.\n";
            close(clientSocket);
            return;
        }

        string request(buffer);
        string response;

        if (request[0] == '1') {
            // handle signup
        } else if (request[0] == '2') {
            // handle signin
        }else if (request[0] == '3') {
            // handle exit
        } else {
            response = "Invalid request!";
        }

        if (write(clientSocket, response.c_str(), response.length()) < 0) {
            cerr << "Error writing to client socket.\n";
        }
    }
    close(clientSocket);
}