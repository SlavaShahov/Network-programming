#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Использование: " << argv[0] << " <IP-адрес> <порт>\n";
        return 1;
    }

    const char* serverIP = argv[1];
    int serverPort = atoi(argv[2]);

    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        perror("Ошибка создания сокета");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    for (int i = 1; i <= 10000000; i++) {
        char buffer[BUFFER_SIZE];
        sprintf(buffer, "%d", i);
        sendto(clientSocket, buffer, strlen(buffer), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

        sockaddr_in fromAddr;
        socklen_t fromAddrLen = sizeof(fromAddr);
        int received = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&fromAddr, &fromAddrLen);
        if (received > 0) {
            buffer[received] = '\0';
            std::cout << "Ответ от сервера: " << buffer << std::endl;
        }

        sleep(i);
    }

    close(clientSocket);
    return 0;
}
