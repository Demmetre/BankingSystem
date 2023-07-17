#include "Storage.h"
#include <cstring>
#include <unordered_map>
#include <random>
#include <thread>
#include <mutex>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <unordered_set>

#define PORT 8080

using namespace std;

unordered_set<string> activeUsernames;
mutex activeUsernamesMutex;

void addActiveUsername(const string& username) {
    lock_guard<mutex> lock(activeUsernamesMutex);
    activeUsernames.insert(username);
}

void removeActiveUsername(const string& username) {
    lock_guard<mutex> lock(activeUsernamesMutex);
    activeUsernames.erase(username);
}

bool isUsernameActive(const string& username) {
    lock_guard<mutex> lock(activeUsernamesMutex);
    return activeUsernames.count(username) > 0;
}

pair<string,string> parseRequest(string req){
    string username = "";
    string password = "";
    int len = req.size(); int i;
    for (i = 2; i < len; i++){
        if (req[i] == ':') break;
        username += req[i];
    }
    i++;
    while(i < len){
        password += req[i];
        i++;
    }
    return make_pair(username, password);
}

void handleClient(int clientSocket) {
    string logedIn = "";
    while(true){
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            cerr << "Error reading from client socket or client disconnected.\n";
            removeActiveUsername(logedIn);
            logedIn = "";
            break; // Break out of the loop to terminate the thread
        }

        string request(buffer);
        string response;

        if (request[0] == '1') {
            // handle signup
            if(logedIn != ""){
                response = "Currently logged in, please log out!\n";
            }else{
                pair<string,string> user = parseRequest(request);
                cout<<user.first<< "  " << user.second <<endl;
                if (signup(user.first, user.second)){
                    response = "Signup successful!\n";
                    addActiveUsername(user.first);
                    logedIn = user.first;
                }else{
                    response = "Username already exists. Please choose a different username.\n";
                }
            }
        } else if (request[0] == '2') {
            if(logedIn != ""){
                response = "Currently logged in, please log out!\n";
            }else{
                pair<string,string> user = parseRequest(request);
                cout<<user.first<< "  " << user.second <<endl;
                if(isUsernameActive(user.first)){
                    response = "This user already logged in!\n";
                }else if (login(user.first, user.second)){
                    response = "Login successful!\n";
                    addActiveUsername(user.first);
                    logedIn = user.first;
                }else{
                    response = "Invalid username or password.\n";
                }
            }
        }else if (request[0] == '3') {
            // handle exit
            if(logedIn == ""){
                response = "Alredy logged out!\n";
            }else{
                response = "Logout successful!\n";
                removeActiveUsername(logedIn);
                logedIn = "";
            }
        } else {
            response = "Invalid request!";
        }

        if (write(clientSocket, response.c_str(), response.length()) < 0) {
            cerr << "Error writing to client socket.\n";
        }
    }
    close(clientSocket);
}


int main() {
    loadCustomersFromFile();
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(struct sockaddr_in);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Failed to create socket!" << endl;
        return 1;
    }

    // Prepare the sockaddr_in structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Failed to bind!" << endl;
        return 1;
    }

    // Listen
    if (listen(serverSocket, 3) < 0) {
        cerr << "Failed to listen!" << endl;
        return 1;
    }

    cout << "Server started. Waiting for incoming connections..." << endl;

    vector<thread> clientThreads;

    // Accept incoming connections and handle clients
    while (true) {
        // Accept connection
        newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (newSocket < 0) {
            cerr << "Failed to accept connection!" << endl;
            return 1;
        }
        thread clientThread(handleClient, newSocket);
        clientThread.detach();
        clientThreads.push_back(move(clientThread));
    }
    close(serverSocket);
    return 0;
}
