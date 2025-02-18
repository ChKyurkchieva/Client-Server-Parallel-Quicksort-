// Client code in C to sort the array
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ws2_32")
#include <iostream>
#include <errno.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <in6addr.h>
#include <WS2tcpip.h>
#include<ws2def.h>
#include <string.h>
#include <io.h>

using std::cin;
using std::cout;
using std::nothrow;
using std::endl;

void fillUp(int* m, int size) {
    cout << "Please, input elements of array: ";
    for (int i = 0; i < size; i++)
        cin >> m[i];
}

unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    r = (rand() % range);
    return min + r;
}

void fillupRandomly(int* m, int size, unsigned int min, unsigned int max) {
    for (int i = 0; i < size; i++)
        m[i] = rand_interval(min, max);
}

// Driver code
int main(int argc, char* argv[])
{

    int N;
    cout << "Please, input size of array: ";
    cin >> N;
    int numThreads;
    cout << "Please, input number of threads: ";
    cin >> numThreads; 

    int* numbers = new int[N];

    fillupRandomly(numbers, N, 0, 100000);
    //fillUp(numbers, N);

    //initialize WSA variables
    WSADATA wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);

    if (wsaerr != 0) {
        std::cout << "The winsock dll not found" << std::endl;
        delete[]numbers;
        return 0;
    }
    else {
        std::cout << "The Winsock dll found" << std::endl;
        std::cout << "The status: " << wsaData.szSystemStatus << std::endl;
    }

    // Create socket
    SOCKET socket_client = INVALID_SOCKET;
    socket_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socket_client == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        delete[]numbers;
        return 0;
    }
    cout << "Socket is created!" << endl;

    sockaddr_in server;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8880);

    // Connect to remote server
    if (connect(socket_client, reinterpret_cast<SOCKADDR*>(&server), sizeof(server)) == SOCKET_ERROR) {
        std::cout << "Client: connect() - Failed to connect: " << WSAGetLastError() << std::endl;
        WSACleanup();
        delete[]numbers;
        return 0;
    }
    else {
        std::cout << "Client: Connect() is OK!" << std::endl;
        std::cout << "Client: Can start sending and receiving data..." << std::endl;
    }

    int sendBytesCount = send(socket_client, (const char*)&N, sizeof(N), 0);
    if (sendBytesCount == SOCKET_ERROR) {
        std::cout << "Client send error: " << WSAGetLastError() << std::endl;
        delete[]numbers;
        return -1;
    }
    std::cout << "Client: Sent " << sendBytesCount << " bytes" << endl;

    sendBytesCount = send(socket_client, (const char*)numbers, N * sizeof(int), 0);
    if (sendBytesCount == SOCKET_ERROR) {
        std::cout << "Client send error: " << WSAGetLastError() << std::endl;
        delete[]numbers;
        return -1;
    }
    cout << "Client: Sent " << sendBytesCount << " bytes" << endl;

    int* receivedNumbers = new int[N];
    int bytesExpected = N * sizeof(int);
    int totalReceived = 0;

    // We cast the int* to a char* to receive raw bytes.
    char* receivedBuffer = (char*)receivedNumbers;

    while (totalReceived < bytesExpected) {
        int bytes = recv(socket_client, receivedBuffer + totalReceived, bytesExpected - totalReceived, 0);
        if (bytes == SOCKET_ERROR) {
            std::cerr << "Error in recv: " << WSAGetLastError() << std::endl;
            break;
        }
        if (bytes == 0) { // connection closed by server
            std::cerr << "Connection closed by server." << std::endl;
            break;
        }
        totalReceived += bytes;
    }

    if (totalReceived < bytesExpected) {
        std::cerr << "Incomplete data received: expected " << bytesExpected << " bytes, but got " << totalReceived << " bytes." << std::endl;
    }
    else {
        std::cout << "Client: Received sorted integers: ";
        for (int i = 0; i < N; i++) {
            std::cout << receivedNumbers[i] << " ";
        }
        std::cout << std::endl;
    }

    delete[]numbers;
    delete[] receivedNumbers;

    WSACleanup();
    return 0;
}