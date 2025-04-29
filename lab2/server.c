#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define BACKLOG 10

void handle_client(int client_socket, int client_id) {
    char buffer[1024];
    int bytes_received;

    while (1) {
        // Чтение данных из сокета
        bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            
            if (bytes_received == 0) {
                printf("Client %d disconnected\n", client_id);
            } else {
                perror("recv");
            }
            break;
        }

        // Добавляем завершающий нулевой символ
        buffer[bytes_received] = '\0';
        printf("Received from client %d: %s\n", client_id, buffer);
    }

    close(client_socket);
    exit(0);
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pid_t pid;
    int client_id = 0;  // Счетчик клиентов

    // Создание сокета
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 0;  // 0 для автоматического выбора порта

    // Привязка сокета к адресу
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Получение номера порта
    getsockname(server_socket, (struct sockaddr *)&server_addr, &client_len);
    printf("Server is running on port: %d\n", ntohs(server_addr.sin_port));

    // Прослушивание входящих соединений
    if (listen(server_socket, BACKLOG) < 0) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Обработка сигналов для завершения зомби-процессов
    signal(SIGCHLD, sigchld_handler);

    while (1) {
        // Принятие соединения от клиента
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("accept");
            continue;
        }

        // Увеличиваем счетчик клиентов
        client_id++;

        // Создание дочернего процесса для обработки клиента
        pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_socket);
        } else if (pid == 0) {
            // Дочерний процесс
            close(server_socket);
            handle_client(client_socket, client_id);
        } else {
            // Родительский процесс
            close(client_socket);
        }
    }

    close(server_socket);
    return 0;
}