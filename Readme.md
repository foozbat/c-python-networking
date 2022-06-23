# `C Server, Python Client Demo`

### Catalog Managment System User Guide

## How to complile
    
Run "make" in the Server root directory.  The server executable will be placed in ./BIN

## How to start the service

To start the catalog server, type "make run" or "./BIN/Server" in the Server root directory.

## How to connect

To start the catalog client, type "python3 Client.py ADDRESS".  ADDRESS is the IP address or hostname of the actively running server.

Once the client has started, you can login using the default credentials of admin / password.

## How to interact with client

Once logged in, the client offers a menu of options to choose from.  You can logout from the server at any time and you will be returned to the Login Menu.

## Known issues

* Server does not automatically recollect expired books.  The provided specification did not implement the expiration date in the packet structure, so this feature was not implemented.
* Server interface has no functionality other than "Quit"
* Server will not function without required datafiles present, nor will it function if datafile field names are not present or incorrect.</li>
