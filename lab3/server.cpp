#include <iostream>
#include <fstream>
#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define BACKLOG 5
#define BUFFER_SIZE 1024

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void *client_handler(void *arg) {
    int client_sock = *(int *)arg;
    delete (int *)arg;

    char buffer[BUFFER_SIZE];
    std::ofstream file("server_log.txt", std::ios::app);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            close(client_sock);
            pthread_exit(nullptr);
        }

        buffer[bytes_received] = '\0';
        std::cout << "Получено: " << buffer << std::endl;

        pthread_mutex_lock(&file_mutex);
        file << buffer << std::endl;
        pthread_mutex_unlock(&file_mutex);
    }

    file.close();
    close(client_sock);
    return nullptr;
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Ошибка создания сокета");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 0;

    if (bind(server_sock, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка bind");
        return 1;
    }

    socklen_t addr_len = sizeof(server_addr);
    if (getsockname(server_sock, (sockaddr *)&server_addr, &addr_len) == -1) {
        perror("Ошибка getsockname");
        return 1;
    }

    std::cout << "Сервер запущен на порту: " << ntohs(server_addr.sin_port) << std::endl;

    if (listen(server_sock, BACKLOG) < 0) {
        perror("Ошибка listen");
        return 1;
    }

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int *client_sock = new int(accept(server_sock, (sockaddr *)&client_addr, &client_len));
        if (*client_sock < 0) {
            perror("Ошибка accept");
            delete client_sock;
            continue;
        }

        pthread_t thread;
        pthread_create(&thread, nullptr, client_handler, client_sock);
        pthread_detach(thread);
    }

    close(server_sock);
    return 0;
}
