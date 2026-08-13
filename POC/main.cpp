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
            std::cout << "> " << std::flush;
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