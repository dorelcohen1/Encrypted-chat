#pragma once

#include <cstddef> // Needed for std::size_t

// -- Default Network Configuration -- //

// Default loopback IP address used for local testing
constexpr const char* IP_ADDRESS = "127.0.0.1";

// Default TCP port used when hosting or connecting locally
constexpr int PORT = 8080;

// -- Network Input Validation Limits -- //

// Minimum valid TCP port number
constexpr int MIN_PORT = 1;

// Maximum valid TCP port number
constexpr int MAX_PORT = 65535;

// Minimum string length for a valid IPv4 address (e.g. "1.1.1.1")
constexpr std::size_t MIN_IPV4_LENGTH = 7;

// Maximum string length for a valid IPv4 address (e.g. "255.255.255.255")
constexpr std::size_t MAX_IPV4_LENGTH = 15;

// Allowed characters inside a standard IPv4 address string
constexpr const char* VALID_IP_CHARS = "0123456789.";

// -- Menu & Connection Prompt Constants -- //

// Main menu option to host a session
constexpr int MENU_HOST = 1;

// Main menu option to connect to a peer
constexpr int MENU_CONNECT = 2;

// Main menu option to exit the application
constexpr int MENU_EXIT = 3;

// Connection target option to use local loopback IP
constexpr int CONNECT_MODE_LOOPBACK = 1;

// Connection target option to manually type peer IP and port
constexpr int CONNECT_MODE_MANUAL = 2;

// Shortcut input value to select the default port
constexpr int USE_DEFAULT_PORT = 0;

// -- IPv4 Parsing Constants -- //

// Minimum numeric value of a single IPv4 octet
constexpr int MIN_IPV4_OCTET = 0;

// Maximum numeric value of a single IPv4 octet
constexpr int MAX_IPV4_OCTET = 255;

// Expected number of octets in an IPv4 address
constexpr std::size_t IPV4_OCTET_COUNT = 4;

// Maximum number of digits allowed in a single IPv4 octet
constexpr std::size_t MAX_OCTET_DIGITS = 3;

// Decimal base used when parsing octet digit characters
constexpr int DECIMAL_BASE = 10;