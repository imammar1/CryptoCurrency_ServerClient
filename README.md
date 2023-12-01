# CryptoCurrency_ServerClient

README for Sender and TCP_Server Solutions
**Overview**
This repository contains two Microsoft Visual Studio solution files:

1. **Sender.sln** 
- This solution is likely to be part of a client application designed to send data.
2. **TCP_Server.sln**
- This solution is expected to be a server application designed to receive and process data sent by the client.
  
**Requirements**
- Microsoft Visual Studio (version used for development should be specified)
- .NET Framework or .NET Core (depending on the project setup)
  
**Setting Up**
- Clone the repository to your local machine.
- Open the solution files in Microsoft Visual Studio.
  
**Sender.sln**
- This solution is expected to contain the client-side logic. It should include projects for the user interface, client-side networking, and any other functionalities specific to the data sending process.

**Key Features**
- Establishes a connection with the server.
- Sends data to the server.
- Handles response or acknowledgments from the server.
  
**TCP_Server.sln**
- This solution contains the server-side application. It manages incoming connections and processes the data received from the client.

**Key Features**
- Listens for incoming client connections.
- Receives and processes data sent by clients.
- Sends response or acknowledgments back to the clients.
  
**Configuration**
- Each solution may contain various configurations for debugging, testing, and production. These configurations can be adjusted within Visual Studio.

**Running the Applications**
- Open the Sender.sln file in Visual Studio and build the project.
- Run the sender/client application.
- Open the TCP_Server.sln file in Visual Studio and build the project.
- Run the server application.
- Ensure both applications are configured to communicate over the same network settings (IP, port, etc.).
