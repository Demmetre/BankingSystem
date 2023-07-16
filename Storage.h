
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


struct Customer {
    std::string name;
    std::string accountNumber;
    std::string hashedPassword;
    double balance;
    // Other member variables and methods as needed
};

void encryptData(const std::string& data, std::string& encryptedData);
void hashPassword(const std::string& password, std::string& hashedPassword);
void saveCustomerToFile(const Customer& customer);
void loadCustomersFromFile();
void signup();
void login();
void getAllCustomers(std::vector<Customer>& result);

#endif
