#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

const int PORT = 8080;
const string SERVER_IP = "127.0.0.1"; // Replace with the actual server IP

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Failed to create socket." << endl;
        return -1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP.c_str(), &(serverAddress.sin_addr)) <= 0) {
        cerr << "Invalid server address." << endl;
        return -1;
    }

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
        cerr << "Connection failed." << endl;
        return -1;
    }

    // At this point, the client is connected to the server
    int choice;
    while(true){
        // Example: Sending a request to the server
        cout << "1. Signup\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        string request;
        switch (choice) {
            case 1:
                request = "1:";
                break;
            case 2:
                request = "2:";
                break;
            case 3:
                request = "3:";
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
        if(choice == 3) break;
        if(choice >= 4 || choice <= 0) continue;
        
        string username, password;
        // Get username and password from user input
        cout << "Enter your username :" ;
        cin >> username;

        cout << "Enter your password :" ;
        cin >> password;

        request += username + ":" + password;

        if (send(clientSocket, request.c_str(), request.length(), 0) < 0) {
            cerr << "Failed to send request." << endl;
            return -1;
        }

        // Example: Receiving response from the server
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        if (recv(clientSocket, buffer, sizeof(buffer) - 1, 0) < 0) {
            cerr << "Failed to receive response." << endl;
            return -1;
        }
        // Process and display the response
        cout << "Response received: " << buffer << endl;
    }
    // Close the connection
    close(clientSocket);

    return 0;
}
