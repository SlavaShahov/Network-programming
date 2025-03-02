#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];
    int i;

    // Создание сокета
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        perror("connect");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Передача данных на сервер
    for (i = 1; i <= 5; i++) {
        snprintf(buffer, sizeof(buffer), "Message %d", i);
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            break;
        }
        printf("Sent: %s\n", buffer);
        sleep(i);  // Задержка в i секунд
    }

    close(client_socket);
    return 0;
}