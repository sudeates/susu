#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CITIES 100 // Saklanabilecek maksimum �ehir say�s�

const char *valid_cities[] = {
    "Adana", "Adiyaman", "Afyonkarahisar", "Agri", "Aksaray", "Amasya", "Ankara", "Antalya", 
    "Ardahan", "Artvin", "Aydin", "Balikesir", "Bartin", "Batman", "Bayburt", "Bilecik", 
    "Bing�l", "Bitlis", "Bolu", "Burdur", "Bursa", "Canakkale", "Cank�r�", "Corum", "Denizli", 
    "Diyarbak�r", "Duzce", "Edirne", "Elazig", "Erzincan", "Erzurum", "Eskisehir", "Gaziantep", 
    "Giresun", "G�m�shane", "Hakkari", "Hatay", "igdir", "isparta", "Istanbul", "�zmir", 
    "Kahramanmara�", "Karab�k", "Karaman", "Kars", "Kastamonu", "Kayseri", "K�r�kkale", 
    "K�rklareli", "K�r�ehir", "Kilis", "Kocaeli", "Konya", "K�tahya", "Malatya", "Manisa", 
    "Mardin", "Mersin", "Mu�la", "Mu�", "Nev�ehir", "Nigde", "Ordu", "Osmaniye", "Rize", 
    "Sakarya", "Samsun", "�anl�urfa", "Siirt", "Sinop", "��rnak", "Sivas", "Tekirda�", 
    "Tokat", "Trabzon", "Tunceli", "U�ak", "Van", "Yalova", "Yozgat", "Zonguldak"
};
const int city_count = sizeof(valid_cities) / sizeof(valid_cities[0]);

const char *weather_conditions[] = {"Sunny", "Rainy", "Cloudy", "Snowy"};
const int weather_count = 4;

// Winsock ba�latma fonksiyonu
void init_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock could not be initialized. Error code: %d\n", WSAGetLastError());
        exit(1);
    }
}

// �ehir do�rulama fonksiyonu
int is_valid_city(const char *city) {
    for (int i = 0; i < city_count; i++) {
        if (strcmp(city, valid_cities[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// �stemciyi i�lemek i�in thread fonksiyonu
DWORD WINAPI handle_client(LPVOID client_socket_ptr) {
    SOCKET client_socket = *(SOCKET *)client_socket_ptr;
    free(client_socket_ptr); // Belle�i serbest b�rak
    char buffer[BUFFER_SIZE];

    while (1) {
        int recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (recv_size == SOCKET_ERROR || recv_size == 0) {
            printf("Client disconnected or error occurred.\n");
            break;
        }

        buffer[recv_size] = '\0';
        printf("Received city: %s\n", buffer);

        // ��k�� komutu kontrol�
        if (strcmp(buffer, "exit") == 0) {
            printf("Client requested to disconnect.\n");
            break;
        }

        // �ehir do�rulama
        if (!is_valid_city(buffer)) {
            const char *error_message = "Invalid city. Please enter a valid Turkish city.";
            send(client_socket, error_message, strlen(error_message), 0);
            continue; // Kullan�c�dan tekrar �ehir girmesi istenecek
        }

        // Rastgele hava durumu tahmini
        srand(time(NULL) + client_socket); // Her istemci i�in farkl� seed
        const char *weather = weather_conditions[rand() % weather_count];

        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "Weather in %s: %s", buffer, weather);

        // Yan�t g�nder
        send(client_socket, response, strlen(response), 0);
    }

    closesocket(client_socket);
    return 0;
}

int main() {
    init_winsock();

    SOCKET server_socket;
    struct sockaddr_in server_addr;
    int client_len;
    struct sockaddr_in client_addr;

    // Sunucu soketi olu�tur
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket could not be created. Error code: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    // Sunucu adresini ayarla
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // SO_REUSEADDR se�ene�i ile soket ba�la
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        printf("Error setting SO_REUSEADDR. Error code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        exit(1);
    }

    // Soketi ba�la
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        exit(1);
    }

    // Dinlemeye ba�la
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        printf("Listen failed. Error code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        exit(1);
    }

    printf("Server is running and waiting for clients...\n");

    while (1) {
        client_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Failed to accept client connection. Error code: %d\n", WSAGetLastError());
            continue;
        }

        printf("Client connected.\n");

        // Yeni istemci i�in bir thread ba�lat
        SOCKET *client_socket_ptr = malloc(sizeof(SOCKET));
        *client_socket_ptr = client_socket;
        CreateThread(NULL, 0, handle_client, client_socket_ptr, 0, NULL);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
