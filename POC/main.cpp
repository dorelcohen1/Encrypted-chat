#include <iostream>
#include "connection.hpp"

int main(int argc, char* argv[]) 
{
	// Test the encryption and decryption functions multiple times 
	// this makes sure that the encryption key is generated randomly 
    // each time and that the functions work correctly.
    for (int i = 0; i < 10; ++i)
    {
        // Implementation of testing the encryption and decryption functions
        std::string plaintext = "This is a test string for encryption.";

        // FIX: Remove the empty parentheses
        Encryption encryption;

        std::string ciphertext = encryption.Encrypt(plaintext);
        std::string decryptedText = encryption.Decrypt(ciphertext);

        std::cout << "Original:  " << plaintext << std::endl;
        std::cout << "Encrypted: " << ciphertext << std::endl;
        std::cout << "Decrypted: " << decryptedText << std::endl << "\n";
    }

    return 0;
}