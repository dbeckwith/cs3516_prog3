Program 3: Sending Photos To A Concurrent Photo Server
======================================================

Authors: Daniel Beckwith (djbeckwith), Aditya Nivarthi (anivarthi)

Purpose:
* This program is used to demonstrate sending and receiving photos from a client to a server.
* The client can send photos separated in packets to the server, which saves them as new files.
* The connection protocols between client and server are simulated to demonstrate their function. The OSI Network Layer, Data Link Layer, and Physical Layer are implemented in the application as a simulation.
  * The Network Layer is responsible for converting chunks of the photos into packets to be sent.
  * The Data Link Layer is responsible for converting the packets into frames to be sent.
  * The Physical Layer is responsible for sending the frame data to the server, and receiving acknowledgement data.

Compile:
* Use make to compile, which will create team_3_photo_client and team_3_photo_server

Execution:
* For the client side, execution would look like:
```
./team_3_photo_client <hostname> <client id> <photo count>
```
* For the server side, execution would look like:
```
./team_3_photo_server
```

Output Files:
* server_<id>.log
  * This file is the log for the server for the client id <id>.
* client_<id>.log
  * This file is the log for the client with id <id>.
* photonew_<id>_<photo num>.jpg
  * This type of file is the new photo produced from the sent packets from the client. The client is defined by id <id> and <photo num> is the nth photo sent by that client.

Performance Timing:
* TODO


