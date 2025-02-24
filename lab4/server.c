#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>

#define BACKLOG 5
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket, port = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    fd_set read_fds, master_fds;
    int max_fd;

    // Создаем серверный сокет
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    // Настраиваем адрес сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(0);  // 0 означает выбор свободного порта

    // Привязываем сокет
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка привязки");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Получаем назначенный порт
    socklen_t len = sizeof(server_addr);
    getsockname(server_fd, (struct sockaddr *)&server_addr, &len);
    port = ntohs(server_addr.sin_port);
    printf("Сервер запущен на порту %d\n", port);

    // Переводим сервер в режим прослушивания
    if (listen(server_fd, BACKLOG) < 0) {
        perror("Ошибка прослушивания");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Инициализируем множества дескрипторов
    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);
    max_fd = server_fd;

    printf("Ожидание подключений...\n");

    while (1) {
        read_fds = master_fds;  // Копируем множество

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Ошибка select");
            break;
        }

        // Проверяем, есть ли новое подключение
        if (FD_ISSET(server_fd, &read_fds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
            if (new_socket < 0) {
                perror("Ошибка accept");
                continue;
            }
            printf("Новое соединение: %s:%d\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            FD_SET(new_socket, &master_fds);
            if (new_socket > max_fd) max_fd = new_socket;
        }

        // Проверяем данные от клиентов
        for (int i = 0; i <= max_fd; i++) {
            if (i != server_fd && FD_ISSET(i, &read_fds)) {
                char buffer[BUFFER_SIZE];
                int bytes_received = recv(i, buffer, sizeof(buffer) - 1, 0);
                if (bytes_received <= 0) {
                    printf("Клиент отключился\n");
                    close(i);
                    FD_CLR(i, &master_fds);
                } else {
                    buffer[bytes_received] = '\0';
                    printf("Получено: %s\n", buffer);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
