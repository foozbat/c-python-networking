# CATALOG CLIENT APPLICATION
# Author:      Aaron Bishop
# Date:        4/19/2020
# Description: Simple procedural command-line client to Catalog Server
# Usage:       -

import sys, getopt
import socket
import struct
from datetime import datetime
from os import system, name
from time import sleep
from getpass import getpass
from pathlib import Path

VERSION = '0.1'

# port to connect to on server
SERVER_PORT = 31337

# port to listen on to receive inventory report
LISTEN_PORT = 13371

# recommend not changing this, these characters were selected because they will not generate a newline
XOR_KEY = "z9xm89c+"  

# DEFINES, do not change
CATALOG_CMD_MAXLEN = 20
CHUNK_SIZE = 1024

# OP CODES, do not change
CATALOG_CMD_CONNECT = 0x10
CATALOG_CMD_ADD_USER = 0x20
CATALOG_CMD_REQUEST_BOOK = 0x30
CATALOG_CMD_ADD_BOOK = 0x40
CATALOG_CMD_REQUEST_REPORT = 0x50
CATALOG_CMD_RETURN_BOOK = 0x60
CATALOG_CMD_GET_AVAILABILITY = 0x70

####################
# HELPER FUNCTIONS #
####################

def print_banner():
    print("-----")
    print("Catalog Managment Tool v", VERSION, sep="")
    print("-----")
    print("")

def exit_disconnected():
    print("Disconnected from server")
    sys.exit()

def clear(): 
    if name == 'nt': 
        _ = system('cls') 
    else: 
        _ = system('clear') 

def xor_crypt(data, key): 
    return ''.join(chr(ord(x) ^ ord(y)) for (x,y) in zip(data, key))

def to_bytes_np(string, num_bytes):
    return bytes(string, "UTF-8") + b"\0"*(num_bytes-len(string))

def build_command(op_code, data):
    return struct.pack("!B", op_code) + data

def send_command(sock, command):
    try:
        sock.sendall(command)
    except:
        exit_disconnected()

####################
# PRIMARY COMMANDS #
####################

# add_user()
# Prompts the user for a username/password, then requests server adds the user to user db
def add_user(sock):
    while True:
        print("")
        print("Add User:")
        print("")
        username = input("Enter new username (11 characters max): ")
        # Prepare ADD USER command 1
        command = build_command(CATALOG_CMD_ADD_USER, to_bytes_np(username, 11))
        send_command(sock, command)
        response = sock.recv(CATALOG_CMD_MAXLEN)

        if len(response) == 0:
            exit_disconnected()
        elif response == b"username_exists":
            print("")
            print("Username is already taken.")
        elif response == b"username_available":
            while True:
                password1 = getpass("Enter new password (8 characters max): ")
                password2 = getpass("Enter password again: ")

                if password1 != password2:
                    print("")
                    print("Password do not match.")
                elif len(password1) > 8:
                    print("")
                    print("Password is too long.")
                else:
                    # Prepare ADD USER command 2
                    command = build_command(CATALOG_CMD_ADD_USER, to_bytes_np(xor_crypt(password1, XOR_KEY), 8))
                    send_command(sock, command)
                    response = sock.recv(CATALOG_CMD_MAXLEN)

                    if len(response) == 0:
                        exit_disconnected()
                    elif response == b"add_user_success":
                        print("")
                        print("New user added.")
                    else:
                        print("New user could not be added.")
                    
                    break
            break

# add_book()
# Prompts the user for a book to add, the requests server add it to catalog db
def add_book(sock):
    while True:
        print("")
        print("Add a New Book (13 characters max)")
        print("")
        book_name = input("Enter the book name: ")
        book_qty = input("Enter the quantity of books to add: ")

        if len(book_name) > 13:
            print("")
            print("Book name is too long.")
            continue
        elif not book_qty.isdigit():
            print("")
            print("Book quantity is not numeric.")
        else:
            command = build_command(CATALOG_CMD_ADD_BOOK, struct.pack("!H", int(book_qty))+to_bytes_np(book_name, 13))
            send_command(sock, command)
            response = sock.recv(CATALOG_CMD_MAXLEN)

            if len(response) == 0:
                exit_disconnected()
            elif response == b"add_book_success":
                print("")
                print("Added",book_qty,"copies of book",book_name,"to the catalog.")
            else:
                print("Book(s) could not be added.")
            break

# request_book()
# Prompts the user for the book name and number of books to request
#  then requests server to update requests db
def request_book(sock):
    while True:
        print("")
        print("Request a Book")
        print("")
        book_name = input("Enter the book name: ")
        book_qty = input("Enter the quantity of books to request: ")

        if len(book_name) > 13:
            print("")
            print("Book name is too long.")
        elif not book_qty.isdigit():
            print("")
            print("Book quantity is not numeric.")    
        else:
            command = build_command(CATALOG_CMD_REQUEST_BOOK, struct.pack("!H", int(book_qty))+to_bytes_np(book_name, 13))
            send_command(sock, command)
            response = sock.recv(CATALOG_CMD_MAXLEN)

            if len(response) == 0:
                exit_disconnected()
            elif response == b"req_book_success":
                print("")
                print("Requested",book_qty,"copies of book",book_name,"from the catalog.")
            else:
                print("Invalid book request.")
            break

# return_book()
# Prompts the user for the book name and number of books to return
#  then requests server to update requests db
def return_book(sock):
    while True:
        print("")
        print("Return a Book")
        print("")
        book_name = input("Enter the book name: ")
        book_qty = input("Enter the quantity of books to return: ")

        if len(book_name) > 13:
            print("")
            print("Book name is too long.")
        elif not book_qty.isdigit():
            print("")
            print("Book quantity is not numeric.")    
        else:
            command = build_command(CATALOG_CMD_RETURN_BOOK, struct.pack("!H", int(book_qty))+to_bytes_np(book_name, 13))
            send_command(sock, command)
            response = sock.recv(CATALOG_CMD_MAXLEN)

            if len(response) == 0:
                exit_disconnected()
            elif response == b"ret_book_success":
                print("")
                print("Returned",book_qty,"copies of book",book_name,"to the catalog.")
            else:
                print("Invalid book return.")
            break

# get_availability()
# Shows the current book availability as specified
def get_availability(sock):
    command = build_command(CATALOG_CMD_GET_AVAILABILITY, b"")
    send_command(sock, command)

    availability_report = ""

    while True:
        response = sock.recv(1024)
        availability_report = availability_report + response.decode("utf-8")
        if len(response) == 0 and availability_report == "":
            exit_disconnected()
        elif len(response) < 1024:
            break

    print("")
    print("Current Availability:")
    print("")
    print(availability_report)

# request_report()
# Requests the server generate an inventory report, then receives the complete report
#  on specified listener port
def request_report(sock):
    print("")
    print("Reqest Inventory Report:")
    print("")

    response = b""
    report_data = b""
    data = b""
    home_dir = str(Path.home())
    report_filename = home_dir + "/Inventory Report - " + datetime.now().strftime("%b %d %Y %H_%M") + ".txt"

    # request report generation
    command = build_command(CATALOG_CMD_REQUEST_REPORT, struct.pack("!H", int(LISTEN_PORT)))
    send_command(sock, command)

    # receive completed report on LISTEN_PORT
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listen_sock:
        listen_sock.bind(("localhost", LISTEN_PORT))
        listen_sock.listen()
        conn, addr = listen_sock.accept()
        with conn:
            print("")
            print("Receving file... ", end="")

            while True:
                data = conn.recv(CHUNK_SIZE)
                report_data = report_data + data
                if len(data) < CHUNK_SIZE:
                    break

            print(len(data),"bytes received")

            send_command(conn, struct.pack("!L", int(len(data))))
            response = conn.recv(CATALOG_CMD_MAXLEN)

    print("")

    # notify user of success/failure
    if len(response) == 0:
        exit_disconnected()
    elif response == b"req_report_success":
        with open(report_filename, "wb") as f:
            f.write(report_data)

        print("Report saved as", report_filename)
    elif response == b"req_filesize_error":
        print("Report could not be saved (file size mismatch)")
    else:
        print("Report could not be saved (unknown error)")

    print("")

################
# MAIN PROGRAM #
################

def main():
    # get server address from command line
    try:
        server_address = sys.argv[1]
    except:
        print("Catalog server address is required")
        print("")
        print("  usage: Capstone.py ADDRESS")
        print("")
        sys.exit()

    # main loop
    while True:
        # open the socket for the conversation
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_addr = (server_address, SERVER_PORT)
            sock.connect(server_addr)
        except:
            print("Could not connect to server:", server_address)
            sys.exit()

        # present login menu
        clear()
        print_banner()
        print("Login Options:")
        print("")
        print("  1. Login.")
        print("  2. Exit.")
        print("")
        option = input("Enter 1-2 to continue: ")

        if option == "2":
            break
        elif option != "1":
            continue
        elif option == "1":
            clear()
            print_banner()
            print("Enter your credentials to login.")
            print("")
            username = input("Username: ")
            password = getpass("Password: ")

            # Prepare CONNECT command
            data = to_bytes_np(username, 11) + to_bytes_np(xor_crypt(password, XOR_KEY), 8)
            command = build_command(CATALOG_CMD_CONNECT,  data)
            send_command(sock, command)
            response = sock.recv(CATALOG_CMD_MAXLEN)

            # all remaining functions require user to be authenticated
            if len(response) == 0:
                exit_disconnected()
            elif response == b"login_success":
                print("")
                print("Login successful.")
                sleep(1)
                clear()

                while True:
                    print_banner()

                    print("Main Menu:")
                    print("")
                    print("  1. Add a new user.")
                    print("  2. View available books.")
                    print("  3. Add a new book.")
                    print("  4. Request a book.")
                    print("  5. Return a book.")
                    print("  6. Request inventory report")
                    print("  7. Logout")
                    print("")
                    option = input("Enter 1-7 to continue: ")

                    # 1. Add a new user
                    if option == "1":
                        add_user(sock)
                        sleep(0.5)
                        clear()
                    # 2. View available books
                    elif option == "2":
                        get_availability(sock)
                    # 3. Add a new book
                    elif option == "3":
                        add_book(sock)
                        sleep(1)
                        clear()
                    # 4. Request a book
                    elif option == "4":
                        request_book(sock)
                        sleep(1)
                        clear()
                    # 5. Return a book
                    elif option == "5":
                        return_book(sock)                            
                        sleep(1)
                        clear()
                    # 6. Request inventory report
                    elif option == "6":
                        request_report(sock)                            
                        sleep(2)
                        clear()
                    # 7. Logout
                    elif option == "7":
                        break
                    else:
                        clear()
                        continue
                
            elif response == b"invalid_user":
                print("")
                print("Invalid user.")
                sleep(1)
            elif response == b"invalid_password":
                print("")
                print("Invalid password.")
                sleep(1)

            # if we got here, login was unsuccessful or user selected "Logout"
            sock.close()

if __name__ == "__main__":
    main()