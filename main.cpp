    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <vector>
    #include <openssl/evp.h>
    #include <openssl/sha.h>
    #include <iomanip>

    struct Customer {
        std::string name;
        std::string accountNumber;
        std::string hashedPassword;
        double balance;
        // Other member variables and methods as needed
    };

    void encryptData(const std::string& data, std::string& encryptedData) {
        const EVP_CIPHER* cipher = EVP_aes_256_cbc();
        unsigned char key[EVP_MAX_KEY_LENGTH] = "myencryptionkey123456789012345"; // 32-byte key
        unsigned char iv[EVP_MAX_IV_LENGTH] = "myinitializer"; // 16-byte IV

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
    
    void hashPassword(const std::string& password, std::string& hashedPassword) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((const unsigned char*)password.c_str(), password.size(), hash);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }

        hashedPassword = ss.str();
    }

    void saveCustomerToFile(const Customer& customer) {
        std::ofstream file("customer.txt", std::ios::app);
        if (!file) {
            std::cerr << "Error opening file for writing.\n";
            return;
        }

        std::string encryptedData;
        encryptData(customer.name + "," + customer.accountNumber + "," + customer.hashedPassword + "," + std::to_string(customer.balance), encryptedData);
        file << encryptedData << '\n';

        file.close();
    }

    void loadCustomersFromFile(std::vector<Customer>& customers) {
        std::ifstream file("customer.txt");
        if (!file) {
            std::cerr << "Error opening file for reading.\n";
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::string decryptedData;

            const EVP_CIPHER* cipher = EVP_aes_256_cbc();
            unsigned char key[EVP_MAX_KEY_LENGTH] = "myencryptionkey123456789012345"; // 32-byte key
            unsigned char iv[EVP_MAX_IV_LENGTH] = "myinitializer"; // 16-byte IV

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

            std::istringstream iss(decryptedData);
            std::string name, accountNumber, hashedPassword;
            double balance;
            if (std::getline(iss, name, ',') && std::getline(iss, accountNumber, ',') && std::getline(iss, hashedPassword, ',') && iss >> balance) {
                customers.push_back({ name, accountNumber, hashedPassword, balance });
            }
        }

        file.close();
    }

    int main() {
        // Example usage to demonstrate saving and loading customer information securely

        // Save customer information
        Customer customer1;
        customer1.name = "John Doe";
        customer1.accountNumber = "1234567890";
        hashPassword("password123", customer1.hashedPassword);
        customer1.balance = 1000.0;

        saveCustomerToFile(customer1);

        // Load customer information
        std::vector<Customer> loadedCustomers;
        loadCustomersFromFile(loadedCustomers);

        // Display loaded customer information
        for (const auto& customer : loadedCustomers) {
            std::cout << "Name: " << customer.name << '\n';
            std::cout << "Account Number: " << customer.accountNumber << '\n';
            std::cout << "Balance: " << customer.balance << '\n';
            std::cout << "----------------------\n";
        }

        return 0;
    }
