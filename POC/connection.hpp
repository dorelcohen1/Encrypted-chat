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
    std::unique_ptr<network> m_net;
    std::unique_ptr<Encryption> m_crypto;

    // Internal helper to exchange the key
    void handle_handshake(bool is_host);
};




// --constructor implementation-- //

Connection::Connection() : m_crypto(std::make_unique<Encryption>())
{
    // Initialize with default network settings first
    m_net = std::make_unique<network>();
}




// --role-based establishment methods-- //

void Connection::establish_as_host()
{
    m_net->accept_connection();
    handle_handshake(true);
}

void Connection::establish_as_client(const std::string& ip, int port)
{
    m_net = std::make_unique<network>(ip, port);
    m_net->connect_to_peer();
    handle_handshake(false);
}



// --internal handshake method-- //

void Connection::handle_handshake(bool is_host)
{
    if (is_host)
    {
        // 1. Get the raw 32-byte key
        std::array<uint8_t, 32> rawKey = m_crypto->GetKey();

        // 2. Convert key to a Hex String (similar to Encrypt logic)
        std::ostringstream hexKey;
        for (uint8_t byte : rawKey) 
        {
            hexKey << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }

        // 3. Add the Header and send
        std::string handshake_msg = "KEY:" + hexKey.str();
        m_net->send_data(handshake_msg);
        std::cout << "[Handshake] Key sent to friend." << std::endl;
    }
    else
    {
        // 1. Wait for the key
        std::string received = m_net->receive_data();

        // 2. Verify the Header
        if (received.length() >= 4 && received.substr(0, 4) == "KEY:")
        {
            std::string hexKey = received.substr(4);
            std::array<uint8_t, 32> sharedKey;

            // 3. Convert Hex back to 32 bytes
            for (size_t i = 0; i < 32; ++i) 
            {
                std::string byteString = hexKey.substr(i * 2, 2);
                sharedKey[i] = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
            }

            // 4. Update our Encryption object with the friend's key
            m_crypto = std::make_unique<Encryption>(sharedKey);
            std::cout << "[Handshake] Key received and synchronized!" << std::endl;
        }
    }
}




// --high-level messaging methods-- //

void Connection::send_secure_msg(const std::string& plaintext)
{
    std::string ciphertext = m_crypto->Encrypt(plaintext);
    m_net->send_data(ciphertext);
}

std::string Connection::receive_secure_msg()
{
    std::string ciphertext = m_net->receive_data();
    if (ciphertext.empty()) 
    { 
        return ""; 
    }

    return m_crypto->Decrypt(ciphertext);
}