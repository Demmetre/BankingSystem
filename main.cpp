#include "Storage.h"
#include "Server.h"

using namespace std;

void printAll(){
    vector<Customer> loadedCustomers;
    getAllCustomers(loadedCustomers);
    // Display loaded customer information
    for (const auto& customer : loadedCustomers) {
        cout << "Name: " << customer.name << '\n';
        cout << "Account Number: " << customer.accountNumber << '\n';
        cout << "Balance: " << customer.balance << '\n';
        cout << "----------------------\n";
    }
}

int main() {
    // Save customer information
    // Load customer information
    loadCustomersFromFile();

    Server server;
    server.start();    

    return 0;
}
