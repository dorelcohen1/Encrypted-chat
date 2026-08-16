
#include <iostream>
#include <thread>
#include <mutex>
#include "connection.hpp"


std::mutex console_mtx; // To keep printing orderly

void message_listener(Connection& conn, bool& running) 
{
    while (running) 
    {
        std::string msg = conn.receive_secure_msg();
        if (!msg.empty()) 
        {
            std::lock_guard<std::mutex> lock(console_mtx);
            std::cout << "\n[Friend]: " << msg << "\n" << "> " << std::flush;
        }
    }
}

int main() 
{
    Connection conn;
    int choice;
    bool running = true;

    std::cout << "1. Host Connection\n2. Connect to Peer\nChoice: ";
    std::cin >> choice;

    if (choice == 1) 
    {
        conn.establish_as_host();
    }
    else
    {
        std::string ip;
        int port;
        std::cout << "Enter IP: "; std::cin >> ip;
        std::cout << "Enter Port: "; std::cin >> port;
        conn.establish_as_client(ip, port);
    }

    std::cin.ignore(); // Clear the newline from buffer
    std::cout << "Secure link established. Start chatting! (type 'exit' to quit)\n";

    // Start background thread to listen for incoming data
    std::thread listener(message_listener, std::ref(conn), std::ref(running));

    // Main UI loop
    while (running) 
    {
        std::string input;
        {
            std::lock_guard<std::mutex> lock(console_mtx);
            std::cout << "[you]: > " << std::flush;
        }
        std::getline(std::cin, input);

        if (input == "exit")
        {
            running = false;
            break;
        }

        if (!input.empty()) 
        {
            conn.send_secure_msg(input);
        }
    }

    if (listener.joinable())
    {
        listener.detach(); // For the PoC we'll detach or stop
    }
    return 0;
}

/*

#include "connection.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[Test] Starting Automated Logic Test...\n";

    // 1. Test Encryption Class
    Encryption enc;
    std::string original = "Top-Secret-Message-123";
    std::string ciphertext = enc.Encrypt(original);
    std::string decrypted = enc.Decrypt(ciphertext);

    if (original == decrypted) 
    {
        std::cout << "[Success] Encryption/Decryption logic is sound.\n";
    }
    else 
    {
        std::cerr << "[Failure] Decryption did not match original!\n";
        return 1;
    }

    // 2. Test Connection Class Initialization
    // This checks if the unique_ptrs (network/crypto) initialize without leaking memory
    try 
    {
        Connection conn;
        std::cout << "[Success] Connection object initialized safely.\n";
    }
    catch (...) 
    {
        std::cerr << "[Failure] Connection object crashed during setup!\n";
        return 1;
    }

    std::cout << "[Test] All automated checks passed. Exiting naturally.\n";
    return 0;
}
*/
