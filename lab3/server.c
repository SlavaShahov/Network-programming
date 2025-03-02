#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);

        // Запись в файл с использованием мьютекса для обеспечения целостности данных
        pthread_mutex_lock(&file_mutex);
        FILE* file = fopen("output.txt", "a");
        if (file) {
            fprintf(file, "%s\n", buffer);
            fclose(file);
        }
        pthread_mutex_unlock(&file_mutex);
    }

    // Клиент отключился
    if (bytes_received == 0) {
        printf("Client disconnected\n");
    } else {
        perror("recv failed");
    }

    close(client_socket);
    free(arg);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;

    // Создание сокета
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 0;  // 0 для автоматического выбора порта

    // Привязка сокета к адресу
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Получение номера порта
    getsockname(server_socket, (struct sockaddr*)&server_addr, &client_len);
    printf("Server is running on port: %d\n", ntohs(server_addr.sin_port));

    // Ожидание подключений
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int* new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        // Создание потока для обработки клиента
        if (pthread_create(&tid, NULL, handle_client, new_sock) != 0) {
            perror("Thread creation failed");
            free(new_sock);
        }
    }

    close(server_socket);
    return 0;
}