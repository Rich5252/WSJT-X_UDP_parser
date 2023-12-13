#include <iostream>
#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <format>
#include <tuple>
#include "WSJTX_Packet.h"
#include "UDP_Server.h"

using namespace std;

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

#define BUFLEN 512
#define PORT 2237
#define SERVER "127.0.0.1"  // or "localhost" - ip address of UDP server


int main()
{
    system("title UDP Server");

    sockaddr_in server, client;

    // initialise winsock
    WSADATA wsa;
    printf("Initialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code: %d", WSAGetLastError());
        exit(0);
    }
    printf("Initialised.\n");

    // create a socket
    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket: %d", WSAGetLastError());
    }
    printf("Socket created.\n");

    // prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER);
    server.sin_port = htons(PORT);

    // bind
    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code: %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("Bind done.");

    while (true)
    {
        printf("\nWaiting for data...\n");
        fflush(stdout);
        char message[BUFLEN] = {};

        // try to receive some data, this is a blocking call
        int message_len;
        int slen = sizeof(sockaddr_in);
        if ((message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen)) == SOCKET_ERROR)
        {
            printf("recvfrom() failed with error code: %d", WSAGetLastError());
            exit(0);
        }

        // print details of the client/peer and the data received
        printf("Received packet from %s:%d Len:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message_len);

/*        for (int i = 0 ; i < message_len; i++)
        {
            printf("%c", message[i]);
        }
        printf("\n");

      //alternatively use this to show zero bytes
        for (int i = 0; i < message_len; i++)
        {
            char c = message[i];
            if (c == 0) { c = (char)"."; }
            printf("%c", c);
        }
*/
        ProcessPacket(message, message_len);



     /*   // server/client example - reply the client with 2the same data
        if (sendto(server_socket, message, strlen(message), 0, (sockaddr*)&client, sizeof(sockaddr_in)) == SOCKET_ERROR)
        {
            printf("sendto() failed with error code: %d", WSAGetLastError());
            return 3;
        }
     */
    }

    closesocket(server_socket);
    WSACleanup();
}

void ProcessPacket(char * message, int message_len)
{
    std::vector<uint8_t> pkt(message, message + message_len);
    size_t idx = 0;
    
    WSJTX_Packet NewPacket( pkt, idx);
    NewPacket.Decode();
    printf("Type: %d\n", NewPacket.PacketType);

    if (NewPacket.PacketType == 0) {
        WSJTX_Heartbeat HeartbeatPacket(pkt, NewPacket.index);
        HeartbeatPacket.Decode();
        // emit UI update signal for heartbeat
    }
    else if (NewPacket.PacketType == 1) {
        WSJTX_Status StatusPacket( pkt, NewPacket.index);
        StatusPacket.Decode();
        std::string msg = StatusPacket.DefaultTXMessage;
        msg = removeTrailingSpaces(msg);
        bool TX = StatusPacket.Transmitting;
        printf("\nTX: %d Freq: %d  TxMessage ", TX, StatusPacket.TxDF);

        for (int i = 0; i < msg.size() && msg[i] != '\0'; i++)
        {
            printf("%c", msg[i]);
        }
        printf("\n\n");
        // emit UI update signal for status
    }
    else if (NewPacket.PacketType == 2) {
        WSJTX_Decode DecodePacket(pkt, NewPacket.index);
        DecodePacket.Decode();
        // Process DecodePacket and emit UI update signal
    }
    else if (NewPacket.PacketType == 3) {
        WSJTX_Erase ErasePacket(pkt, NewPacket.index);
        ErasePacket.Decode();
        // emit UI erase message signal
    }
    else if (NewPacket.PacketType == 4) {
        WSJTX_Reply ReplyPacket(pkt, NewPacket.index);
        ReplyPacket.Decode();
        // Process ReplyPacket and emit UI update signal
    }
    else if (NewPacket.PacketType == 5) {
        WSJTX_Logged LoggedPacket(pkt, NewPacket.index);
        LoggedPacket.Decode();
        // emit UI update signal for logged packet
    }
}

std::string removeTrailingSpaces(std::string str) {
    // Find the position of the last non-space character
    auto pos = str.find_last_not_of(' ');

    // Erase all trailing spaces
    if (pos != std::string::npos) {
        str.erase(pos + 1);
    }
    else {
        // The string contains only spaces
        str.clear();
    }
    return str;
}