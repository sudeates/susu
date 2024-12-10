#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define SERVER_IP "127.0.0.1" // Sunucu IP adresi (localhost)
#define PORT 8080
#define BUFFER_SIZE 1024

// Winsock baþlatma fonksiyonu
void init_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock could not be initialized. Error code: %d\n", WSAGetLastError());
        exit(1);
    }
}

int main() {
    // Winsock baþlat
    init_winsock();

    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Soket oluþtur
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket could not be created. Error code: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    // Sunucu adresini ayarla
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Sunucuya baðlan
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection to server failed. Error code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        exit(1);
    }

    printf("Connected to the server.\n");

    while (1) {
        // Kullanýcýdan giriþ al
        printf("Enter a city name (or 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Satýr sonu karakterini kaldýr
        buffer[strcspn(buffer, "\n")] = '\0';

        // Çýkýþ kontrolü
        if (strcmp(buffer, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }

        // Veriyi sunucuya gönder
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            printf("Data could not be sent. Error code: %d\n", WSAGetLastError());
            break;
        }

        // Sunucudan yanýt al
        int recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (recv_size == SOCKET_ERROR) {
            printf("Failed to receive data. Error code: %d\n", WSAGetLastError());
            break;
        }

        buffer[recv_size] = '\0';
        printf("Server response: %s\n", buffer);
    }

    // Soketi kapat
    closesocket(client_socket);
    WSACleanup();
    return 0; // Ana fonksiyonu doðru þekilde bitiriyoruz
}
