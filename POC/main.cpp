#include <iostream>
#include <thread>
#include <mutex>
#include <atomic> // Added for thread-safe flags
#include <limits> // added for std::numeric_limits and Minimum and maximum values
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

// Reads an integer from std::cin safely and clears leftover buffer/error state
bool read_int_safely(int& out_value)
{
    bool is_valid = false;

    // Attempt to read an integer from standard input
    if (std::cin >> out_value)
    {
        is_valid = true;
    }
    else
    {
        // Clear stream error flags when non-numeric input is entered
        std::cin.clear();
        is_valid = false;
    }

    // Flush the rest of the input line including the trailing newline
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    return is_valid;
}

// Validates an IPv4 string for length, allowed characters, and 4 octets (0-255)
bool is_valid_ipv4(const std::string& ip)
{
    std::size_t index = 0;
    std::size_t octet_count = 0;
    std::size_t octet_digits = 0;
    int octet_value = 0;
    bool has_leading_zero = false;
    char current_char = '\0';

    // Check overall IPv4 string length bounds
    if (ip.length() < MIN_IPV4_LENGTH || ip.length() > MAX_IPV4_LENGTH)
    {
        return false;
    }

    // Reject characters outside digits and dots
    if (ip.find_first_not_of(VALID_IP_CHARS) != std::string::npos)
    {
        return false;
    }

    // Reject leading or trailing dots
    if (ip[0] == '.' || ip[ip.length() - 1] == '.')
    {
        return false;
    }

    // Walk through each character to validate dot placement and octet values
    for (index = 0; index < ip.length(); ++index)
    {
        current_char = ip[index];

        if (current_char == '.')
        {
            // Reject empty octets from consecutive dots
            if (octet_digits == 0)
            {
                return false;
            }

            // Commit octet and reset counters for the next octet
            octet_count += 1;
            octet_digits = 0;
            octet_value = 0;
            has_leading_zero = false;
        }
        else
        {
            // Reject multi-digit octets with a leading zero (e.g. "01")
            if (octet_digits == 0 && current_char == '0')
            {
                has_leading_zero = true;
            }
            else if (has_leading_zero)
            {
                return false;
            }

            // Verify digit count per octet does not exceed 3
            octet_digits += 1;
            if (octet_digits > MAX_OCTET_DIGITS)
            {
                return false;
            }

            // Build octet integer value and verify 0-255 bounds
            octet_value = (octet_value * DECIMAL_BASE) + (current_char - '0');
            if (octet_value < MIN_IPV4_OCTET || octet_value > MAX_IPV4_OCTET)
            {
                return false;
            }
        }
    }

    // Ensure the last octet was non-empty and total count equals 4
    if (octet_digits == 0)
    {
        return false;
    }

    octet_count += 1;
    if (octet_count != IPV4_OCTET_COUNT)
    {
        return false;
    }

    return true;
}

// Prompts the user for a valid TCP port or 0 to select the default port
int prompt_for_port(const std::string& prompt_label)
{
    int port = 0;
    bool valid_port = false;

    // Loop until a valid port or default shortcut is entered
    while (!valid_port)
    {
        std::cout << prompt_label << " (" << MIN_PORT << "-" << MAX_PORT << ", or " << USE_DEFAULT_PORT << " for default " << PORT << "): ";

        if (!read_int_safely(port))
        {
            std::cout << "Invalid input. Please enter a numeric port." << std::endl;
            continue;
        }

        if (port == USE_DEFAULT_PORT)
        {
            port = PORT;
            valid_port = true;
        }
        else if (port < MIN_PORT || port > MAX_PORT)
        {
            std::cout << "Invalid port. Please enter a number between " << MIN_PORT << " and " << MAX_PORT << "." << std::endl;
        }
        else
        {
            valid_port = true;
        }
    }

    return port;
}

// Prompts the user to pick loopback or manually enter a peer IPv4 and port
void prompt_for_connection_target(std::string& out_ip, int& out_port)
{
    int connect_mode = 0;
    bool valid_mode = false;
    bool valid_ip = false;

    // Ask whether to connect over local loopback or to a remote peer IP
    while (!valid_mode)
    {
        std::cout << "\nConnection Target:\n" << CONNECT_MODE_LOOPBACK << ". Loopback (" << IP_ADDRESS << ")\n" << CONNECT_MODE_MANUAL << ". Enter Peer IP & Port\nChoice: ";

        if (!read_int_safely(connect_mode))
        {
            std::cout << "Invalid input. Please enter " << CONNECT_MODE_LOOPBACK << " or " << CONNECT_MODE_MANUAL << "." << std::endl;
            continue;
        }

        if (connect_mode == CONNECT_MODE_LOOPBACK)
        {
            out_ip = IP_ADDRESS;
            valid_mode = true;
        }
        else if (connect_mode == CONNECT_MODE_MANUAL)
        {
            // Prompt until a strictly valid IPv4 address is provided
            while (!valid_ip)
            {
                std::cout << "Enter Peer IP: ";
                std::getline(std::cin, out_ip);

                if (!is_valid_ipv4(out_ip))
                {
                    std::cout << "Invalid IP address. Please enter a valid IPv4 address." << std::endl;
                }
                else
                {
                    valid_ip = true;
                }
            }

            valid_mode = true;
        }
        else
        {
            std::cout << "Invalid choice. Please enter " << CONNECT_MODE_LOOPBACK << " or " << CONNECT_MODE_MANUAL << "." << std::endl;
        }
    }

    // Prompt for the destination port after IP selection
    out_port = prompt_for_port("Enter Peer Port");
}


int main()
{
    while (true) // Infinite loop to allow returning to menu
    {
        Connection conn;
        int choice = 0;
        int host_port = PORT;
        std::string ip = "";
        int port = PORT;

        std::cout << "\n--- ENCRYPTED P2P CHAT ---\n" << MENU_HOST << ". Host\n" << MENU_CONNECT << ". Connect\n" << MENU_EXIT << ". Exit\nChoice: ";

        if (!read_int_safely(choice))
        {
            std::cout << "Invalid menu choice. Please enter a number." << std::endl;
            continue;
        }

        if (choice == MENU_EXIT)
        {
            break;
        }

        if (choice == MENU_HOST)
        {
            // Let the host pick a listening port (or 0 for default 8080)
            host_port = prompt_for_port("Enter Port to listen on");

            if (!conn.establish_as_host(host_port))
            {
                continue;
            }
        }
        else if (choice == MENU_CONNECT)
        {
            // Ask for loopback vs manual IP, plus destination port
            prompt_for_connection_target(ip, port);

            if (!conn.establish_as_client(ip, port))
            {
                continue;
            }
        }
        else
        {
            std::cout << "Invalid choice. Please select " << MENU_HOST << ", " << MENU_CONNECT << ", or " << MENU_EXIT << "." << std::endl;
            continue;
        }

        running = true;
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