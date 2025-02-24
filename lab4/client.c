#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void send_messages(int sock, int num) {
    char buffer[256];
    while (1) {
        snprintf(buffer, sizeof(buffer), "Число: %d", num);
        send(sock, buffer, strlen(buffer), 0);
        printf("Отправлено: %s\n", buffer);
        sleep(num);  // Задержка перед следующей отправкой
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Использование: %s <IP> <порт> <число>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int num = atoi(argv[3]);

    if (num < 1 || num > 10) {
        printf("Число должно быть от 1 до 10\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Ошибка создания сокета");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Ошибка преобразования IP-адреса");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка подключения");
        close(sock);
        return 1;
    }

    printf("Подключено к серверу %s:%d\n", server_ip, port);
    send_messages(sock, num);

    close(sock);
    return 0;
}
