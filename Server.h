
#include "Storage.h"
#include <cstring>
#include <unordered_map>
#include <random>
#include <thread>
#include <mutex>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>


using namespace std;

class Server{
    private:
        int serverSocket;
        vector<thread> clientThreads;
        mutex customersMutex;

    public:
        Server();
        ~Server();
        void start();
        void handleClient(int clientSocket);
};