import socket
import threading
import time
import random
import signal
import sys

# List of potential IRC commands to simulate random client behavior
irc_commands = [
    lambda client_id: f"NICK testuser{client_id}\r\n",
    lambda client_id: f"USER testuser{client_id} 0 * :Realname{client_id}\r\n",
    lambda client_id: f"JOIN #channel{random.randint(1, 10)}\r\n",
    lambda client_id: f"JOIN #channel\r\n",
    lambda client_id: f"PART #channel{random.randint(1, 10)}\r\n",
    lambda client_id: f"PART #channel\r\n",
    lambda client_id: f"PRIVMSG #channel{random.randint(1, 10)} :Hello from testuser{client_id}\r\n",
    lambda client_id: f"PRIVMSG #channel :Hello from testuser{client_id}\r\n",
    lambda client_id: "PING :server\r\n",
]

# First command to send, i.e., password
def pass_command():
    return "PASS 42irc\r\n"  # Change to the actual password you use

# Store the status of each client
client_status = {}

# Global flag to signal when to stop the clients
running = True

# Signal handler for SIGQUIT or other termination signals
def handle_sigquit(signum, frame):
    global running
    print("Received signal to stop. Exiting...")
    running = False

# Display client status interactively
def display_status():
    while running:
        print("\033c", end="")  # Clear the console
        print("Client Status:")
        for client_id, status in client_status.items():
            print(f"Client {client_id}: {status}")
        time.sleep(1)

def random_irc_client_behavior(client_id):
    global client_status
    global running
    try:
        irc_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        irc_socket.connect(("localhost", 6667))  # Adjust IP and port as necessary
        client_status[client_id] = "Connected"
        
        # Send the PASS command first
        password_cmd = pass_command()
        client_status[client_id] = "Sending password"
        irc_socket.sendall(password_cmd.encode())

        # Keep performing actions until the program is stopped
        while running:
            command = random.choice(irc_commands)(client_id)
            client_status[client_id] = f"Sending: {command.strip()}"
            irc_socket.sendall(command.encode())

            # Random sleep between actions to simulate staggered activity
            time.sleep(random.uniform(0.1, 1.5))

            # Receive response from server (optional)
            try:
                response = irc_socket.recv(1024).decode()
                client_status[client_id] = f"Received: {response.strip()}"
            except socket.timeout:
                pass

    except Exception as e:
        client_status[client_id] = f"Error: {e}"
    finally:
        irc_socket.close()

def run_multiple_clients(num_clients=200):
    threads = []

    # Start the display thread
    display_thread = threading.Thread(target=display_status)
    display_thread.daemon = True  # This thread will exit when the main thread exits
    display_thread.start()

    for client_id in range(1, num_clients + 1):
        thread = threading.Thread(target=random_irc_client_behavior, args=(client_id,))
        threads.append(thread)
        thread.start()

    # Let all threads run in parallel
    for thread in threads:
        thread.join()

if __name__ == "__main__":
    # Set up signal handling for SIGQUIT (Ctrl+\) or SIGINT (Ctrl+C)
    signal.signal(signal.SIGQUIT, handle_sigquit)
    signal.signal(signal.SIGINT, handle_sigquit)

    # Infinite loop until signal is received
    run_multiple_clients(200)
