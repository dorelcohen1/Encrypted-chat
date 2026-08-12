// for networking and our p2p conaction
#include "include/asio.hpp"
#include <iostream>
#include "hard_numbers.hpp"

class network
{
public:
    network();
    ~network();

    void send_data(const std::string& msg);
    void start_listening();

private:
    // Core Asio engine objects live safely inside the class private zone
    asio::io_context io_ctx_;
    asio::ip::tcp::acceptor acceptor_;
    asio::ip::tcp::socket out_socket_;
    asio::ip::tcp::socket incoming_socket_;
};

// Constructor: Initialize the Asio context and bind the listener to your PORT constant
network::network() : acceptor_(io_ctx_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), PORT)), out_socket_(io_ctx_),incoming_socket_(io_ctx_)
{

}

network::~network()
{
    // Clean up connections when the class is destroyed
    io_ctx_.stop();
}

void network::send_data(const std::string& msg)
{
    try 
    {
        // Only connect if the outbound pipe isn't open yet
        if (!out_socket_.is_open()) 
        {
            asio::ip::tcp::endpoint peer_addr(asio::ip::make_address(IP_ADDRESS), PORT);
            out_socket_.connect(peer_addr);
            std::cout << "[Network] Connected to peer successfully." << std::endl;
        }

        // Send the raw characters over the wire
        asio::write(out_socket_, asio::buffer(msg));
        std::cout << "Message sent successfully!" << std::endl;
    }
    catch (std::exception& e) 
    {
        std::cerr << "[Send Error]: " << e.what() << std::endl;
    }
}

void network::start_listening()
{
    try 
    {
        std::cout << "Waiting for friend to connect on port " << PORT << "..." << std::endl;

        // Block the thread until someone connects
        acceptor_.accept(incoming_socket_);
        std::cout << "Friend connected!" << std::endl;

        // Read the incoming bytes into a text buffer
        char data_buffer[1024] = { 0 };
        size_t bytes_received = incoming_socket_.read_some(asio::buffer(data_buffer));

        std::cout << "Received: " << std::string(data_buffer, bytes_received) << std::endl;
    }
    catch (std::exception& e) 
    {
        std::cerr << "[Listen Error]: " << e.what() << std::endl;
    }
}
