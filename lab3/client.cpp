#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Использование: " << argv[0] << " <IP> <PORT> <COUNT>\n";
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = std::stoi(argv[2]);
    int count = std::stoi(argv[3]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Ошибка создания сокета");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Ошибка преобразования IP");
        return 1;
    }

    if (connect(sock, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка подключения");
        return 1;
    }

    for (int i = 1; i <= count; ++i) {
        std::string message = std::to_string(i);
        send(sock, message.c_str(), message.length(), 0);
        std::cout << "Отправлено: " << message << std::endl;
        sleep(i);
    }

    close(sock);
    return 0;
}
