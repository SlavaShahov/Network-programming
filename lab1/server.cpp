#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main() {
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        perror("Ошибка создания сокета");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = 0; // Автоматический выбор порта

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Ошибка привязки сокета");
        return 1;
    }

    socklen_t addrLen = sizeof(serverAddr);
    getsockname(serverSocket, (sockaddr*)&serverAddr, &addrLen);
    std::cout << "Сервер запущен на порту: " << ntohs(serverAddr.sin_port) << std::endl;

    char buffer[BUFFER_SIZE];
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    while (true) {
        int received = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
        if (received > 0) {
            buffer[received] = '\0';
            int num = atoi(buffer);
            std::cout << "Получено: " << num << " от " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
            num *= 2;
            sprintf(buffer, "%d", num);
            sendto(serverSocket, buffer, strlen(buffer), 0, (sockaddr*)&clientAddr, clientAddrSize);
        }
    }

    close(serverSocket);
    return 0;
}
