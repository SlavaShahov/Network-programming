#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Использование: %s <IP> <PORT> <COUNT>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int count = atoi(argv[3]);

    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Ошибка в IP-адресе");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Ошибка подключения");
        exit(1);
    }

    for (int i = 1; i <= count; i++) {
        int num = htonl(i);
        if (write(sock, &num, sizeof(num)) == -1) {
            perror("Ошибка отправки данных");
            break;
        }
        printf("Отправлено: %d\n", i);
        sleep(i);
    }

    close(sock);
    return 0;
}
