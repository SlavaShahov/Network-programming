#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    int i;

    // Создание сокета
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Преобразование IP-адреса
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Подключение к серверу
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Enter a number between 1 and 10: ");
    scanf("%d", &i);

    if (i < 1 || i > 10) {
        printf("Invalid number. Please enter a number between 1 and 10.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Отправка данных на сервер
    while (1) {
        snprintf(buffer, BUFFER_SIZE, "%d", i);
        send(sock, buffer, strlen(buffer), 0);
        printf("Sent: %s\n", buffer);
        sleep(i); // Задержка
    }

    close(sock);
    return 0;
}