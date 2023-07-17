#include "Storage.h"
#include <cstring>
#include <unordered_map>
#include <random>
#include <mutex>

using namespace std;

const char kEncryptionKey[] = "myencryptionkey123456789012345";
const char kInitializationVector[] = "myinitializer";
const double INITIAL_BALANCE = 1000.0;

unordered_map<string, Customer> customers;
mutex dataMutex;

void encryptData(const string& data, string& encryptedData) {
    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
    unsigned char key[EVP_MAX_KEY_LENGTH]; 
    unsigned char iv[EVP_MAX_IV_LENGTH];

    memcpy(key, kEncryptionKey, sizeof(kEncryptionKey)); 
    memcpy(iv, kInitializationVector, sizeof(kInitializationVector));
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, cipher, nullptr, key, iv);

    int maxOutputLength = data.size() + EVP_CIPHER_block_size(cipher);
    encryptedData.resize(maxOutputLength);

    int encryptedLength = 0;
    EVP_EncryptUpdate(ctx, (unsigned char*)&encryptedData[0], &encryptedLength, (const unsigned char*)data.c_str(), data.size());

    int finalEncryptedLength = 0;
    EVP_EncryptFinal_ex(ctx, (unsigned char*)&encryptedData[encryptedLength], &finalEncryptedLength);

    encryptedLength += finalEncryptedLength;
    encryptedData.resize(encryptedLength);

    EVP_CIPHER_CTX_free(ctx);
}

void hashPassword(const string& password, string& hashedPassword) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)password.c_str(), password.size(), hash);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }

    hashedPassword = ss.str();
}

void saveCustomerToFile(const Customer& customer) {
    ofstream file("customer.txt", ios::app);
    if (!file) {
        cerr << "Error opening file for writing.\n";
        return;
    }

    string encryptedData;
    encryptData(customer.name + "," + customer.accountNumber + "," + customer.hashedPassword + "," + to_string(customer.balance), encryptedData);
    file << encryptedData << '\n';

    file.close();
}

void loadCustomersFromFile() {
    ifstream file("customer.txt");
    if (!file) {
        cerr << "Error opening file for reading.\n";
        return;
    }

    string line;
    while (getline(file, line)) {
        string decryptedData;

        const EVP_CIPHER* cipher = EVP_aes_256_cbc();
        unsigned char key[EVP_MAX_KEY_LENGTH];
        unsigned char iv[EVP_MAX_IV_LENGTH];

        memcpy(key, kEncryptionKey, sizeof(kEncryptionKey)); 
        memcpy(iv, kInitializationVector, sizeof(kInitializationVector));

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        EVP_DecryptInit_ex(ctx, cipher, nullptr, key, iv);

        decryptedData.resize(line.size() + EVP_MAX_BLOCK_LENGTH);
        int decryptedLength = 0;
        EVP_DecryptUpdate(ctx, (unsigned char*)&decryptedData[0], &decryptedLength, (const unsigned char*)line.c_str(), line.size());

        int finalDecryptedLength = 0;
        EVP_DecryptFinal_ex(ctx, (unsigned char*)&decryptedData[decryptedLength], &finalDecryptedLength);

        decryptedLength += finalDecryptedLength;
        decryptedData.resize(decryptedLength);

        EVP_CIPHER_CTX_free(ctx);

        istringstream iss(decryptedData);
        string name, accountNumber, hashedPassword;
        double balance;

        if (getline(iss, name, ',') && getline(iss, accountNumber, ',') && getline(iss, hashedPassword, ',') && iss >> balance) {
            Customer c ;
            c.name = name;
            c.accountNumber = accountNumber;
            c.balance = balance;
            c.hashedPassword = hashedPassword;
            customers[name] = c;
        }
    }

    file.close();
}
void getAllCustomers(vector<Customer>& result){
    for (auto it: customers){
        result.push_back(it.second);
    }
}
string generateRandomAccountNumber() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(10000000, 99999999);
    return to_string(dist(gen));
}

bool login(string username, string password) {
    lock_guard<mutex> lock(dataMutex);
    // Check if the username already exists
    // if (customers.find(username) == customers.end()) {
    //     return false;
    // }

    // Hash the entered password
    string hashedPassword;
    hashPassword(password, hashedPassword);

    Customer it = customers[username];
    // Compare the hashed password with the stored hashed password
    if (hashedPassword == it.hashedPassword) {
        // Proceed with account operations
        return true;
    } else {
        return false;
    }
}

bool signup(string username, string password) {

    lock_guard<mutex> lock(dataMutex);
    // Check if the username already exists
    if (customers.find(username) != customers.end()) {
        return false;
    }

    // Hash the password
    string hashedPassword;
    hashPassword(password, hashedPassword);

    // Create a new user record
    Customer newUser;
    newUser.name = username;
    newUser.hashedPassword = hashedPassword;
    newUser.balance = INITIAL_BALANCE;
    newUser.accountNumber = generateRandomAccountNumber();

    saveCustomerToFile(newUser);
    customers[username] = newUser;  

    return true;
}