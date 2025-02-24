#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define BACKLOG 5  // Максимальное число ожидающих соединений

void handle_client(int client_sock) {
    int num;
    while (read(client_sock, &num, sizeof(num)) > 0) {
        printf("Получено число: %d\n", ntohl(num));
        sleep(1);  // Имитируем обработку
    }
    close(client_sock);
    exit(0);
}

void reap_zombies(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 0; // Автоматический выбор свободного порта

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Ошибка при привязке сокета");
        exit(1);
    }

    // Получаем и выводим назначенный порт
    socklen_t len = sizeof(server_addr);
    if (getsockname(server_sock, (struct sockaddr *)&server_addr, &len) == -1) {
        perror("Ошибка getsockname");
        exit(1);
    }
    printf("Сервер слушает на порту: %d\n", ntohs(server_addr.sin_port));

    if (listen(server_sock, BACKLOG) == -1) {
        perror("Ошибка при прослушивании");
        exit(1);
    }

    signal(SIGCHLD, reap_zombies); // Обработка зомби-процессов

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        if (client_sock == -1) {
            perror("Ошибка при принятии соединения");
            continue;
        }

        printf("Новое соединение\n");

        pid_t pid = fork();
        if (pid == -1) {
            perror("Ошибка fork");
            close(client_sock);
            continue;
        }

        if (pid == 0) { // Дочерний процесс
            close(server_sock);
            handle_client(client_sock);
        } else {
            close(client_sock); // Родитель закрывает сокет клиента
        }
    }

    close(server_sock);
    return 0;
}
