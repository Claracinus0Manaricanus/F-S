# F-S
This a basic server client model used for sending or receiving from/to server.

How To Use?
First run the server it will start on port 34576 as a tcp server.
Connect the client by writing the port and ip of server.
after connection client will ask you to enter a number (0 or 1).
0 means client will send a file. 1 means client will receive a file.
Both will lead to client waiting for a filename.
According to the choice made you will receive or send the entered file. (file names cannot exceed 20 characters)
Only one file can be sent or received from/to server. After the transaction both server and client will close.

Supported systems:
GNU/Linux (tested)

plans)
better tui
multiple transfers
