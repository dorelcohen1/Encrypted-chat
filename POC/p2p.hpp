// for networking and our p2p conaction
#include "include/asio.hpp"
#include <iostream>
#include "hard_numbers.hpp"

class network
{
public:
    network(const std::string& ip, int p);
    network();
    ~network();

    bool send_data(const std::string& msg);
    std::string receive_data();

    bool accept_connection();
    bool connect_to_peer();

    bool is_connected() const;

private:

    int port = PORT; // Use the constant from hard_numbers.hpp
    std::string ip_address = IP_ADDRESS; // Use the constant from hard_numbers.hpp

    // Core Asio engine objects live safely inside the class private zone
    asio::io_context io_ctx_;
    asio::ip::tcp::acceptor acceptor_;

	// Use a smart pointer for the session socket to manage its lifetime automatically
    std::unique_ptr<asio::ip::tcp::socket> session_socket_;
};





// --constructor and destructor implementations-- //

// Parameterized Constructor
network::network(const std::string& ip, int p) : port(p), ip_address(ip), acceptor_(io_ctx_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
    // session_socket_ starts as nullptr automatically
}

// Default Constructor
network::network() : acceptor_(io_ctx_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{

}

network::~network()
{
    // Clean up connections when the class is destroyed
    io_ctx_.stop();
}




// --send and receive methods-- //

bool network::send_data(const std::string& msg)
{
    try
    {
        // Check if we actually have a connection first
        if (session_socket_ && session_socket_->is_open())
        {
            asio::write(*session_socket_, asio::buffer(msg));
        }
        else
        {
            std::cerr << "[Send Error]: No active connection found!" << std::endl;
            return false;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "[Send Error]: " << e.what() << std::endl;
        return false;
    }
    return true;
}

std::string network::receive_data()
{
    try
    {
        if (session_socket_ && session_socket_->is_open())
        {
            char data_buffer[1024] = { 0 };
            asio::error_code ec; // Use error_code to prevent exceptions on disconnect
            size_t bytes_received = session_socket_->read_some(asio::buffer(data_buffer), ec);

            if (ec == asio::error::eof || ec == asio::error::connection_reset)
            {
                session_socket_->close(); // Explicitly close on our end
                return "";
            }

            return std::string(data_buffer, bytes_received);
        }
    }
    catch (std::exception& e)
    {
        if (session_socket_)
        {
            session_socket_->close();
        }
		std::cerr << "[Receive Error]: " << e.what() << std::endl;
    }
    return "";
}





// --connection methods-- //

bool network::accept_connection()
{
    try
    {
        // 1. Initialize the socket inside the smart pointer
		session_socket_ = std::make_unique<asio::ip::tcp::socket>(io_ctx_); // we need a pointer because acceptor_.accept() requires a reference to a socket
		// fill the pointer with a new socket object

        std::cout << "[Network] Waiting for a friend on port " << port << "..." << std::endl;

        // 2. Block until someone connects, placing the connection into our smart-pointer socket
		acceptor_.accept(*session_socket_); // give the reference of the socket to acceptor_.accept() so it can fill it with the new connection

        std::cout << "[Network] Friend connected from: " << session_socket_->remote_endpoint().address().to_string() << std::endl;

		return true;
    }
    catch (std::exception& e)
    {
        std::cerr << "[Accept Error]: " << e.what() << std::endl;
		return false;
    }
}

bool network::connect_to_peer()
{
    try
    {
        // 1. Initialize the socket
        session_socket_ = std::make_unique<asio::ip::tcp::socket>(io_ctx_);

        // 2. Resolve the address and connect
        asio::ip::tcp::endpoint peer_addr(asio::ip::make_address(ip_address), port);
        session_socket_->connect(peer_addr);

        std::cout << "[Network] Successfully connected to " << ip_address << ":" << port << std::endl;
		return true;
    }
    catch (std::exception& e)
    {
        std::cerr << "[Connect Error]: " << e.what() << std::endl;
		return false;
    }

}

bool network::is_connected() const
{
    return session_socket_ && session_socket_->is_open();
}