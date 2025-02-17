// Server code in C to sort the array
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ws2_32")

#include <io.h>
#include <iostream>
#include <Winsock2.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <in6addr.h>
#include <WS2tcpip.h>
#include<ws2def.h>
#include <string.h>

void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

//Partitioning, using partiotion algorithm by Lomuto
int partition(int arr[], int low, int high)
{
    int pivot = arr[high]; // pivot
    int i = (low - 1); // Index of smaller element
    for (int j = low; j <= high - 1; j++)
    {
        if (arr[j] <= pivot)
        {
            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quicksort(int* a, int low, int high)
{
    int div;

    if (low < high) {
        div = partition(a, low, high);
#pragma omp task shared(a) if(high - low > TASK_SIZE) 
        quicksort(a, low, div - 1);
#pragma omp task shared(a) if(high - low > TASK_SIZE)
        quicksort(a, div + 1, high);
    }
}

// Driver code
int main(int argc, char* argv[])
{
    //initialization
    WSADATA wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);

    // Check for initialization success
    if (wsaerr != 0) {
        std::cout << "The Winsock dll not found!" << std::endl;
        return 0;
    }
    else {
        std::cout << "The Winsock dll found" << std::endl;
        std::cout << "The status: " << wsaData.szSystemStatus << std::endl;
    }

    // Create socket
    SOCKET socket_server = INVALID_SOCKET;

    socket_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


    if (socket_server == INVALID_SOCKET)
    {
        std::cout << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }
    else
    {
        std::cout << "Socket is created!" << std::endl;
    }

    // Prepare the sockaddr_in structure
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(8880);

    // Bind the socket
    if (bind(socket_server, (struct sockaddr*)&service, sizeof(service)) == SOCKET_ERROR) {

        std::cout << "Binding failed" << WSAGetLastError() << std::endl;
        closesocket(socket_server);
        WSACleanup();
        return 0;
    }
    std::cout << "Binding is successful!" << std::endl;

    // listen to the socket
    if (listen(socket_server, 1) == SOCKET_ERROR)
        std::cout << "Listening failed! " << WSAGetLastError() << std::endl;

    std::cout << "Waiting for incoming connections..." << std::endl;

    // accept connection from an incoming client
    SOCKET socket_client = INVALID_SOCKET;
    socket_client = accept(socket_server, nullptr, nullptr);

    if (socket_client == INVALID_SOCKET) {
        std::cout << "Accept faild!" << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    std::cout << "Connection accepted" << std::endl;

    int N = 0;
    int nbytes = recv(socket_client, (char*)&N, sizeof(N), 0);
    if (nbytes != sizeof(N)) {
        std::cerr << "Error: did not receive the expected size." << std::endl;
        // Handle error...
        closesocket(socket_client);
        WSACleanup();
        return -1;
    }

    int* message = new int[N];
    int read_size = recv(socket_client, (char*)message, N * (sizeof(int)), 0);
    if (read_size < 0)
    {
        std::cout << "Server receive failed!" << WSAGetLastError() << std::endl;
        delete[]message;
    }

    quicksort(message, 0, N - 1);

    for (int i = 0; i < N; i++)
    {
        std::cout << message[i] << ' ';
    }

    int sendBytesCount = send(socket_client, (const char*)message, N * sizeof(int), 0);
    if (sendBytesCount == SOCKET_ERROR)
    {
        std::cout << "Server send error: " << WSAGetLastError() << std::endl;
        delete[]message;
        return -1;
    }
    std::cout << "Server: Sent " << sendBytesCount << " bytes" << std::endl;

    delete[] message;

    return 0;
}