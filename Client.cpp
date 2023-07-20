#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define SERVER_IP "127.0.0.1"
#define PORT 8080

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Failed to create socket!" << endl;
        return 1;
    }

    // Prepare the sockaddr_in structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0) {
        cerr << "Invalid address or address not supported!" << endl;
        return 1;
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Failed to connect to the server!" << endl;
        return 1;
    }

    cout << "Connected to the server." << endl;

    // At this point, the client is connected to the server
    int choice;
    while(true){
        // Example: Sending a request to the server
        cout << "1. Signup\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        cout << "4. Money transfer\n";
        cout << "5. Current balance\n";
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
            case 4:
                request = "4:";
                break;
            case 5:
                request = "5:";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
        
        if(choice >= 6 || choice <= 0) break;
        
        string username, password;
        // Get username and password from user input
        if(choice < 3){
            cout << "Enter your username :" ;
            cin >> username;

            cout << "Enter your password :" ;
            cin >> password;
            request += username + ":" + password;
        }
        if(choice == 4){
            cout << "Enter receiver's username :" ;
            cin >> username;
            cout << "Enter amount of money :" ;
            int money; cin>>money;
            request += username + ":" + to_string(money);
        }


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
        cout << ">> " <<buffer << endl;
    }
    // Close the connection
    close(clientSocket);

    return 0;
}
