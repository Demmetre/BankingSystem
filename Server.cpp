#include "Storage.h"
#include <cstring>
#include <unordered_map>
#include <random>
#include <thread>
#include <mutex>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unordered_set>
#include <csignal>
#include <atomic>


#define PORT 8080

using namespace std;


unordered_set<string> activeUsernames;
mutex activeUsernamesMutex;


// Global flag to indicate the server should shut down

std::atomic<bool> g_shutdownRequested(false);

// Signal handler function for SIGINT
void handleSIGINT(int signal) {
    g_shutdownRequested = true;
}

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

void handleSignUp(string& logedIn, string& response, string& request){
    if(logedIn != ""){
        response = "Currently logged in, please log out!\n";
    }else{
        pair<string,string> user = parseRequest(request);
        if (signup(user.first, user.second)){
            response = "Signup successful!\n";
            addActiveUsername(user.first);
            logedIn = user.first;
        }else{
            response = "Username already exists. Please choose a different username.\n";
        }
    }
}

void handleLogIn(string& logedIn, string& response, string& request){
    if(logedIn != ""){
        response = "Currently logged in, please log out!\n";
    }else{
        pair<string,string> user = parseRequest(request);
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
}

void handleExit(string& logedIn, string& response, string& request){
    if(logedIn == ""){
        response = "Alredy logged out!\n";
    }else{
        response = "Logout successful!\n";
        removeActiveUsername(logedIn);
        logedIn = "";
    }
}

void handleMoneyTransfer(string& logedIn, string& response, string& request){
    if(logedIn == ""){
        response = "No current user. Please log in to transfer money\n";
    }else{
        pair<string,string> user = parseRequest(request);
        int res = transferMoney(logedIn, user.first, stoi(user.second));
        if(res == 1){
            response = "User not found.\n";
        }else if (res == 2){
            response = "Not enough money\n";
        }else{
            response = "Transaction completed.\n";
        }
    }
}

void handleCurrentBalance(string& logedIn, string& response, string& request){
    if(logedIn == ""){
        response = "No current user. Please log in to view the balance\n";
    }else{
        int res = getCurrentBalance(logedIn);
        if(res == -1){
            response = "Something went wrong while checking balance\n";
        }else{
            response = "Your current balance is " + to_string(res) + " USD\n";
        }
    }
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
            break;
        }
        string request(buffer);
        string response;

        if (request[0] == '1') {
            handleSignUp(logedIn, response, request);
        } else if (request[0] == '2') {
            handleLogIn(logedIn, response, request);
        }else if (request[0] == '3') {
            handleExit(logedIn, response, request);
        }else if(request[0] == '4'){
            handleMoneyTransfer(logedIn, response, request);
        }else if (request[0] == '5'){
            handleCurrentBalance(logedIn, response, request);
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
    
    signal(SIGINT, handleSIGINT);

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
    createUserDataDirectory();
    loadCustomersFromFile();
    cout << "Server started. Waiting for incoming connections..." << endl;
    vector<thread> clientThreads;

    fd_set readSet;
    int maxfd = serverSocket + 1;

    // Accept incoming connections and handle clients
    while (!g_shutdownRequested) {
        // Accept connection
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);

        // Set a timeout to check for signals
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100 milliseconds

        // Use select to wait for incoming connections or a signal
        int ready = select(maxfd, &readSet, NULL, NULL, &timeout);
        if (ready < 0) {
            break;
        }
        if (ready > 0) {
            newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
            if (newSocket < 0) {
                cerr << "Failed to accept connection!" << endl;
                return 1;
            }
            thread clientThread(handleClient, newSocket);
            clientThread.detach();
            clientThreads.push_back(move(clientThread));
        }
        if (g_shutdownRequested) {
            break;
        }
    }

    cout<<"\nClosing server..."<<endl;
    close(serverSocket);
    return 0;
}
