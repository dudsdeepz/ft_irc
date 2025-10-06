ft_irc
ft_irc is an educational project from the 42 Network curriculum, challenging students to implement a fully functional IRC (Internet Relay Chat) server from scratch using C++98. IRC is a classic, text-based protocol for real-time communication, enabling users to join channels, send public and private messages, and interact in a networked environment.
Overview
This project simulates a single IRC server (the simplest form of an IRC network) that handles multiple client connections concurrently. It emphasizes low-level network programming concepts like sockets, non-blocking I/O, and multiplexing (e.g., using select() or poll()) to manage real-time messaging without threads. The server adheres to core IRC protocol specifications (RFC 1459 and RFC 2812), supporting essential commands for authentication, channel management, and message broadcasting.
Key Goals:

Understand TCP socket programming and client-server architecture.
Implement real-time, multi-user chat functionality.
Handle edge cases like disconnections, invalid inputs, and signal management.

The server can be tested with standard IRC clients like irssi, HexChat, or even netcat (nc).
Features

User Management: NICK, USER commands for registration; supports nickname changes and user modes.
Channel Operations: JOIN, PART, KICK, INVITE; creates and manages public/private channels with operators.
Messaging: PRIVMSG for private/direct messages; channel broadcasting.
Server Commands: MOTD (Message of the Day), PING/PONG for keep-alives, QUIT for graceful exits.
Non-Blocking I/O: Efficiently polls multiple sockets for incoming data without blocking.
Error Handling: Numeric reply codes (e.g., ERR_NOSUCHNICK, RPL_WELCOME) for protocol compliance.
Optional Extensions: Basic bot integration or file transfer hooks (beyond minimum requirements).

Note: No server-to-server linking is implemented, focusing on a standalone server.
Technologies

Language: C++98 (strict compliance for portability).
Libraries: Standard C++ (no external dependencies), <sys/socket.h>, <netinet/in.h>, <poll.h> or <sys/select.h>.
Build: Makefile with g++ compiler.
Testing: Compatible with RFC-compliant IRC clients; automated tests via custom scripts.


