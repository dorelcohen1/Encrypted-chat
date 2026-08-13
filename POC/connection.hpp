#include "encryption.hpp"
#include "p2p.hpp"
#include "hard_numbers.hpp"

class Connection
{
public:
    Connection();

    // Role-based establishment
    void establish_as_host();
    void establish_as_client(const std::string& ip, int port);

    // High-level messaging
    void send_secure_msg(const std::string& plaintext);
    std::string receive_secure_msg();

private:
    network m_net;
    std::unique_ptr<Encryption> m_crypto;

    // Internal helper to exchange the key
    void handle_handshake(bool is_host);
};

connection::Connection() : m_crypto(std::make_unique<Encryption>()) 
{

}
