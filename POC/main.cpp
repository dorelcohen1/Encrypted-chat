#include <iostream>
#include <thread>
#include <mutex>
#include <atomic> // Added for thread-safe flags
#include "connection.hpp"

std::mutex console_mtx;
std::atomic<bool> running{ false }; // Changed to atomic
// atomic is used here to ensure that the 'running' flag can be safely read and written by multiple threads without data races.

void message_listener(Connection& conn)
{
    while (running)
    {
        std::string msg = conn.receive_secure_msg();

        // Check if connection died during receive
        if (msg.empty() && !conn.is_alive())
        {
            std::lock_guard<std::mutex> lock(console_mtx);
            std::cout << "\n[System]: Connection lost. Press Enter to return to menu." << std::endl;
            running = false; // This tells the main thread to stop
            break;
        }

        if (!msg.empty())
        {
            std::lock_guard<std::mutex> lock(console_mtx);
            std::cout << "\r[Friend]: " << msg << std::endl;
            std::cout << "[you   ]: " << std::flush;
        }
    }
}

int main()
{
    while (true) // Infinite loop to allow returning to menu
    {
        Connection conn;
        int choice;

        std::cout << "\n--- ENCRYPTED P2P CHAT ---\n1. Host\n2. Connect\n3. Exit\nChoice: ";

        if (!(std::cin >> choice) || choice == 3)
        {
            break;
        }

        if (choice == 1) 
        {
            if (!conn.establish_as_host())
            {
                continue;
            }
        }
        else 
        {
            std::string ip;
            int port;
			bool valid_input = false;

            while (!valid_input)
            {
                std::cout << "Enter IP: "; std::cin >> ip;
                std::cout << "Enter Port: "; std::cin >> port;

				if (port < 1 || port > 65535)
				{
					std::cout << "Invalid input. Please enter a valid IP and port number (1-65535)." << std::endl;
				}
				else if (ip.empty() || ip.length() > 15 || ip.length() < 7)// test for right size
                {
                    std::cout << "Invalid input. Please enter a valid IP address." << std::endl;
                }
				else if (ip.find_first_not_of("0123456789.") != std::string::npos) // check for invalid characters
                {
					std::cout << "Invalid input. Please enter a valid IP address." << std::endl;
                }
				else
				{
					valid_input = true;
				}
            }

            if (!conn.establish_as_client(ip, port))
            {
                continue;
            }
        }

        running = true;
        std::cin.ignore();
        std::cout << "Secure link established! (type 'menu' to go back)\n\n\n";

        std::thread listener(message_listener, std::ref(conn));

        while (running)
        {
            // Prompt printing
            {
                std::lock_guard<std::mutex> lock(console_mtx);
                std::cout << "[you   ]: " << std::flush;
            }

            std::string input;
            std::getline(std::cin, input);

            // If listener thread killed 'running' while we were waiting for input
            if (!running)
            {
                break;
            }

            if (input == "menu" || input == "exit") 
            {
                running = false;
                if (!conn.stop())
                {
                    std::cerr << "[Error] Failed to stop connection." << std::endl;
					return 1;
                }
                break;
            }

            if (!input.empty()) 
            {
                if (!conn.send_secure_msg(input)) 
                {
                    running = false;
                    break;
                }
            }
        }

        if (listener.joinable())
        {
            listener.join();
        }

        std::cout << "[System]: Returning to Main Menu..." << std::endl;
    }

    return 0;
}