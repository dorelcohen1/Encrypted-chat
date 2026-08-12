// for data structures:
#include <array>   // this is needed for std::array
#include <string>  // this header is needed for std::string
#include <sstream> // this header is needed for std::ostringstream
// for bit control:
#include <cstdint> // this header is needed for uint8_t
#include <random>  // this header is needed for std::random_device, std::mt19937, and std::uniform_int_distribution
#include <iomanip> // this header is needed for std::setw and std::setfill

class Encryption 
{
public:

    Encryption() : m_encryptionKey{}
    {
        // since this is only a POC we will implement a simple  //
        // encryption algorithm instead of making aes-256       //
        // for the poc we will implment Xor-256                 //

        // -- set the key to a random 32-byte value --

        // 1. Hardware-based entropy source for random seeds
        std::random_device rd;

        // 2. Mersenne Twister pseudo-random generator engine seeded with rd
        std::mt19937 generator(rd());

        // 3. Uniform distribution producing byte values from 0x00 to 0xFF (0 to 255)
        std::uniform_int_distribution<uint16_t> distribution(0, 255);

        // 4. Fill each byte of the 32-byte key
        for (size_t i = 0; i < Encryption::m_encryptionKey.size(); ++i)
        {
            Encryption::m_encryptionKey[i] = static_cast<uint8_t>(distribution(generator));
        }
    }

    /*
    * the encryption logic is as follows:
	*   get the length of the plaintext then loop over it
	*   perform an XOR operation between the plaintext byte and the key byte using ^
	*   use the i % m_encryptionKey.size() to wrap around the key if the plaintext is longer than the key 
    *   this looping happens becuse the reminder of dividing by its size will be the corent placment untill it 
	*   is of the same aize and then the reminder will be 0 and it will start over again
	*   we then use std::hex to convert the byte to a hex string and append it to the output string
	*   then we set it to 2 char only using std::setw(2) and fill it with 0s using std::setfill('0') to 
	*   make sure it is always 2 char long and then we return the hex string as the output of the function
    */
    std::string Encrypt(const std::string& plaintext)
    {
		// The Encrypt function takes a plaintext string and returns an encrypted string in hexadecimal format.
        std::ostringstream hexStream;

		// Iterate over each byte of the plaintext
        for (size_t i = 0; i < plaintext.size(); ++i)
        {
            // 1. Perform XOR operation
            uint8_t cipherByte = static_cast<uint8_t>(plaintext[i]) ^ m_encryptionKey[i % m_encryptionKey.size()];

            // 2. Format byte as a 2-character hex string (e.g. 0x05 -> "05", 0xAB -> "ab")
            hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cipherByte);
        }

        return hexStream.str(); // Clean, printable string safe for sockets
    }
    
    /*
	* the decryption logic is as follows:
	*   we loop over the length of the hexCiphertext in steps of 2 because each byte is represented by 2 hex characters
	*   we use substr to get the 2 hex characters and then we use strtol to convert it back to a byte
	*   then we perform the XOR operation with the key byte to retrieve the original plaintext byte
    */
    std::string Decrypt(const std::string& hexCiphertext)
    {
        // The Decrypt function takes a hexadecimal string (representing the encrypted data) and returns the original plaintext string.
        std::string plaintext = "";

		// Iterate over the hexCiphertext in steps of 2 characters (1 byte)
        for (size_t i = 0; i < hexCiphertext.length(); i += 2)
        {
            // 1. Extract 2 hex characters and parse back to a single byte
            std::string hexByte = hexCiphertext.substr(i, 2);
            uint8_t cipherByte = static_cast<uint8_t>(strtol(hexByte.c_str(), nullptr, 16));

            // 2. XOR with key to retrieve original byte
            size_t byteIndex = i / 2;
            char originalChar = static_cast<char>(cipherByte ^ m_encryptionKey[byteIndex % m_encryptionKey.size()]);

            plaintext += originalChar;
        }

        return plaintext;
    }

private:
    std::array<uint8_t, 32> m_encryptionKey;
};

/*
================================================================================
                           CODE EXPLANATION & NOTES
================================================================================

1. std::array<uint8_t, 32>
--------------------------------------------------------------------------------
- A fixed-size array holding 32 unsigned 8-bit integers (bytes).
- 32 bytes * 8 bits/byte = 256 bits total.
- Matches the exact 256-bit key size required by AES-256, making
  it seamless to swap out this PoC cipher for real AES later without changing
  our code structure.

2. std::random_device rd;
--------------------------------------------------------------------------------
- A hardware-based entropy source (calls OS/CPU registers).
- Uses physical thermal noise or micro-timing variances in CPU
  circuits to produce a truly unpredictable seed.

3. std::mt19937 generator(rd());
--------------------------------------------------------------------------------
- A Pseudo-Random Number Generator (PRNG) using the Mersenne
  Twister algorithm.
- True hardware entropy (`std::random_device`) is slow. We use
  `rd()` to seed `generator()`, which quickly expands that initial randomness
  into a reliable stream of pseudo-random numbers.

4. std::uniform_int_distribution<uint16_t> distribution(0, 255);
--------------------------------------------------------------------------------
- A mathematical filter that constrains generated numbers to the
  range [0, 255] (the exact byte range 0x00 to 0xFF).
- Raw PRNG numbers are large integers. This guarantees every
  possible byte value has an equal statistical probability of being picked.
- generator() produces a random integer, and distribution(generator) maps it into the byte range.

5. static_cast<uint8_t>(distribution(generator))
--------------------------------------------------------------------------------
- Safely converts the integer produced by the distribution back
  into a raw single byte (`uint8_t`) before placing it into our key array.

6. Hexadecimal Conversion in Encrypt() and Decrypt()
--------------------------------------------------------------------------------
- XOR encryption produces raw binary bytes (including null bytes
  `0x00`). If we sent raw bytes over a network socket, the OS or socket parser
  might treat `0x00` as an end-of-string character and cut off the message.
- We convert every byte into 2 readable hex characters (e.g.,
  `0x00` -> `"00"`, `0x0F` -> `"0f"`) for safe socket transmission, then parse
  it back to raw bytes on the receiving side.
================================================================================
*/