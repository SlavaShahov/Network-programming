#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    fd_set read_fds;
    int max_sd, activity, i, valread, sd;
    char buffer[BUFFER_SIZE];

    // Инициализация массива клиентских сокетов
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // Создание серверного сокета
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(0); // 0 для автоматического выбора порта

    // Привязка сокета к адресу
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Получение номера порта
    getsockname(server_socket, (struct sockaddr *)&server_addr, &client_len);
    printf("Server is running on port: %d\n", ntohs(server_addr.sin_port));

    // Ожидание подключений
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Очистка набора файловых дескрипторов
        FD_ZERO(&read_fds);

        // Добавление серверного сокета в набор
        FD_SET(server_socket, &read_fds);
        max_sd = server_socket;

        // Добавление клиентских сокетов в набор
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &read_fds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Ожидание активности на одном из сокетов
        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Select error");
        }

        // Если активен серверный сокет, это новое подключение
        if (FD_ISSET(server_socket, &read_fds)) {
            int new_socket;
            if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            // Добавление нового сокета в массив
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    printf("New client connected, socket fd: %d\n", new_socket);
                    break;
                }
            }
        }

        // Обработка данных от клиентов
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            if (FD_ISSET(sd, &read_fds)) {
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // Клиент отключился
                    getpeername(sd, (struct sockaddr *)&client_addr, &client_len);
                    printf("Client disconnected, ip: %s, port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    // Вывод полученных данных
                    buffer[valread] = '\0';
                    printf("Received from client %d: %s\n", sd, buffer);
                }
            }
        }
    }

    return 0;
}