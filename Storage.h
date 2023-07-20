
#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

using namespace std;

struct Customer {
    string name;
    string accountNumber;
    string hashedPassword;
    double balance;
    // Other member variables and methods as needed
};

void encryptData(const string& data, string& encryptedData);
void hashPassword(const string& password, string& hashedPassword);
void saveCustomerToFile(const Customer& customer);
void loadCustomersFromFile();
bool signup(string username, string password);
bool login(string username, string password);
void getAllCustomers(vector<Customer>& result);
int transferMoney(string senderUsername, string receiverUsername, double amount);
double getCurrentBalance(string username);

#endif
