#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define SERVER_IP "127.0.0.1"  // IP-адрес сервера
#define SERVER_PORT 12345       // Порт сервера
#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Создание сокета
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", server_ip, server_port);

    // Отправка данных на сервер
    for (int i = 1; i <= 5; i++) {
        snprintf(buffer, BUFFER_SIZE, "Message %d", i);
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Sent: %s\n", buffer);
        sleep(i);  // Задержка в i секунд
    }

    close(client_socket);
    return 0;
}