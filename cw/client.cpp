#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <thread>

using namespace std;

void clearLine() {
    cout << "\r" << string(100, ' ') << "\r";
}

void readHandler(int sock) {
    char buffer[1024] = {0};
    while (true) {
        int valread = read(sock, buffer, sizeof(buffer)-1);
        if (valread > 0) {
            buffer[valread] = '\0';
            clearLine();
            cout << buffer;
            cout << "> " << flush;
        } else if (valread == 0) {
            cout << "\nСоединение с сервером разорвано\n";
            exit(0);
        } else {
            perror("Ошибка чтения");
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Использование: " << argv[0] << " <IP> <порт>\n";
        return 1;
    }

    string server_ip = argv[1];
    int server_port = atoi(argv[2]);
    string name;

    cout << "Приветвуем на сервере!\n";
    cout << "Введите ваше имя: ";
    getline(cin, name);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Ошибка создания сокета");
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "Неверный адрес: " << server_ip << endl;
        close(sock);
        return -1;
    }

    cout << "Подключаемся к " << server_ip << ":" << server_port << "..." << endl;
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Ошибка подключения");
        close(sock);
        return -1;
    }

    send(sock, name.c_str(), name.length(), 0);

    thread reader(readHandler, sock);
    reader.detach();

    string message;
    while (true) {
        cout << "> " << flush;
        getline(cin, message);
        
        if (message == "/exit") {
            break;
        }
        
        send(sock, message.c_str(), message.length(), 0);
    }

    close(sock);
    return 0;
}
