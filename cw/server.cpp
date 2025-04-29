#include <iostream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <cstring>
#include <algorithm>
#include <map>
#include <set>
#include <fstream>
#include <chrono>
#include <iomanip>

#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024

using namespace std;

struct Client {
    int socket;
    string name;
    string current_room;
};

struct Room {
    string name;
    set<int> clients;
};

map<string, Room> rooms;
map<int, Client> clients;
set<string> used_names;//для использованных имён

ofstream log_file("server_log.txt", ios::app);

//для получения текущей даты и времени в строковом формате
string getCurrentTime() {
    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void logMessage(const string& message) {
    if (log_file.is_open()) {
        log_file << "[" << getCurrentTime() << "] " << message << endl;
    }
}

void sendToClient(int client_socket, const string& message) {
    send(client_socket, message.c_str(), message.size(), 0);
}

void broadcastMessage(const string& room_name, const string& message, int exclude_socket = -1) {
    if (rooms.find(room_name) == rooms.end()) return;
    
    for (int client_socket : rooms[room_name].clients) {
        if (client_socket != exclude_socket) {
            sendToClient(client_socket, message);
        }
    }
}

vector<string> splitCommand(const string& command) {
    vector<string> args;
    string current;
    for (char c : command) {
        if (c == ' ') {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        args.push_back(current);
    }
    return args;
}


void handleCommand(int client_socket, const string& command) {
    Client& client = clients[client_socket];
    vector<string> args = splitCommand(command);
    
    if (args.empty()) return;

    if (args[0] == "/create" && args.size() > 1) {
        string room_name = args[1];
        if (rooms.find(room_name) == rooms.end()) {
            rooms[room_name] = {room_name, {}};
            logMessage(client.name + " создал комнату: " + room_name);
            cout << "[" << getCurrentTime() << "] " << client.name << " создал комнату: " << room_name << endl;
            sendToClient(client_socket, "\033[32mКомната '" + room_name + "' создана.\033[0m\n");
    
            
            if (!client.current_room.empty()) {
                rooms[client.current_room].clients.erase(client_socket);
                broadcastMessage(client.current_room, client.name + " покинул комнату.\n");
            }
    
            client.current_room = room_name;
            rooms[room_name].clients.insert(client_socket);
            logMessage(client.name + " автоматически присоединился к комнате: " + room_name);
            cout << "[" << getCurrentTime() << "] " << client.name << " автоматически присоединился к комнате: " << room_name << endl;
            broadcastMessage(room_name, client.name + " присоединился к комнате.\n");
            sendToClient(client_socket, "\033[32mВы вошли в комнату '" + room_name + "'.\033[0m\n");
    
        } else {
            sendToClient(client_socket, "\033[31mКомната '" + room_name + "' уже существует.\033[0m\n");
        }
        return;
    }
    
    else if (args[0] == "/join" && args.size() > 1) {
        string room_name = args[1];
        if (rooms.find(room_name) != rooms.end()) {
            if (!client.current_room.empty()) {
                rooms[client.current_room].clients.erase(client_socket);
                broadcastMessage(client.current_room, client.name + " покинул комнату.\n");
            }
            client.current_room = room_name;
            rooms[room_name].clients.insert(client_socket);
            logMessage(client.name + " присоединился к комнате: " + room_name);
            cout << "[" << getCurrentTime() << "] " << client.name << " присоединился к комнате: " << room_name << endl;
            broadcastMessage(room_name, client.name + " присоединился к комнате.\n");
            sendToClient(client_socket, "\033[32mВы вошли в комнату '" + room_name + "'.\033[0m\n");
        } else {
            sendToClient(client_socket, "\033[31mКомната '" + room_name + "' не найдена.\033[0m\n");
        }
        return;
    }
    else if (args[0] == "/leave") {
        if (!client.current_room.empty()) {
            string room_name = client.current_room;
            rooms[room_name].clients.erase(client_socket);
            logMessage(client.name + " покинул комнату: " + room_name);
            cout << "[" << getCurrentTime() << "] " << client.name << " покинул комнату: " << room_name << endl;
            broadcastMessage(room_name, client.name + " покинул комнату.\n");
            client.current_room = "";
            sendToClient(client_socket, "\033[32mВы вышли из комнаты.\033[0m\n");
        } else {
            sendToClient(client_socket, "\033[31mВы не находитесь в комнате.\033[0m\n");
        }
        return;
    }
    else if (args[0] == "/list") {
        string response = "\033[34mСписок комнат (" + to_string(rooms.size()) + "):\033[0m\n";
        for (const auto& room : rooms) {
            response += "\033[32m- " + room.first + " (" + to_string(room.second.clients.size()) + " участников)\033[0m\n";
        }
        sendToClient(client_socket, response);
        return;
    }
    else if (args[0] == "/users") {
        if (!client.current_room.empty()) {
            string response = "\033[34mУчастники комнаты '" + client.current_room + "':\033[0m\n";
            for (int socket : rooms[client.current_room].clients) {
                response += "\033[32m- " + clients[socket].name + "\033[0m\n";
            }
            sendToClient(client_socket, response);
        } else {
            sendToClient(client_socket, "\033[31mВы не находитесь в комнате.\033[0m\n");
        }
        return;
    }
    else if (args[0] == "/users_on_server") {
        string response = "\033[34mСписок пользователей на сервере:\033[0m\n";
        if (clients.empty()) {
            response += "\033[31mНа сервере нет подключенных пользователей.\033[0m\n";
        } else {
            for (const auto& client_entry : clients) {
                const Client& c = client_entry.second;
                string room = c.current_room.empty() ? "Не в комнате" : c.current_room;
                response += "\033[32m- " + c.name + " (комната: " + room + ")\033[0m\n";
            }
        }
        sendToClient(client_socket, response);
        return;
    }
    else if (args[0] == "/help") {
        string help_msg = 
            "\033[34mДоступные команды:\033[0m\n"
            "\033[32m/create <название>\033[0m - создать комнату\n"
            "\033[32m/join <название>\033[0m   - войти в комнату\n"
            "\033[32m/leave\033[0m              - выйти из комнаты\n"
            "\033[32m/list\033[0m               - список комнат\n"
            "\033[32m/users\033[0m              - список участников текущей комнаты\n"
            "\033[32m/users_on_server\033[0m   - список всех пользователей на сервере\n"
            "\033[32m/pm <пользователь> <сообщение>\033[0m - отправить личное сообщение\n"
            "\033[32m/help\033[0m               - справка\n";
        sendToClient(client_socket, help_msg);
        return;
    }
    else if (args[0] == "/pm" && args.size() > 2) {
        string target_name = args[1];
        string message = command.substr(command.find(' ', command.find(' ') + 1) + 1);
        bool user_found = false;
        for (const auto& client_entry : clients) {
            if (client_entry.second.name == target_name) {
                sendToClient(client_entry.first, "\033[33m[Личное сообщение от " + client.name + "]: " + message + "\033[0m\n");
                sendToClient(client_socket, "\033[33m[Личное сообщение для " + target_name + "]: " + message + "\033[0m\n");
                logMessage(client.name + " отправил личное сообщение пользователю " + target_name + ": " + message);
                cout << "[" << getCurrentTime() << "] " << client.name << " отправил личное сообщение пользователю " << target_name << ": " << message << endl;
                user_found = true;
                break;
            }
        }
        if (!user_found) {
            sendToClient(client_socket, "\033[31mПользователь с именем '" + target_name + "' не найден.\033[0m\n");
        }
        return;
    }
    else {
        if (!client.current_room.empty()) {
            string formatted_msg = client.name + ": " + command + "\n";
            broadcastMessage(client.current_room, formatted_msg, client_socket);
            logMessage(client.name + " отправил сообщение в комнату '" + client.current_room + "': " + command);
            cout << "[" << getCurrentTime() << "] " << client.name << " отправил сообщение в комнату '" << client.current_room << "': " << command << endl;
        } else {
            sendToClient(client_socket, "\033[31mВы не в комнате. Используйте /join или /create.\033[0m\n");
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Определение порта
    const int PORT = 8080; 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT); 
    //привязка к сокету адреса и порта
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    logMessage("Сервер запущен на порту " + to_string(PORT));
    cout << "Сервер запущен на порту " << PORT << "." << endl;  // Используем переменную PORT в выводе

    fd_set readfds;
    vector<int> client_sockets(MAX_CLIENTS, 0);

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            int valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                close(new_socket);
                continue;
            }

            string client_name(buffer, valread);
            client_name.erase(remove(client_name.begin(), client_name.end(), '\n'), client_name.end());
            client_name.erase(remove(client_name.begin(), client_name.end(), '\r'), client_name.end());

            if (used_names.count(client_name) > 0) {
                string error_msg = "\033[31mИмя '" + client_name + "' уже занято. Пожалуйста, переподключитесь с другим именем.\033[0m\n";
                send(new_socket, error_msg.c_str(), error_msg.size(), 0);
                close(new_socket);
                logMessage("Попытка подключения с уже занятым именем: " + client_name);
                cout << "[" << getCurrentTime() << "] " << "Попытка подключения с уже занятым именем: " << client_name << endl;
                continue;
            }

            clients[new_socket] = {new_socket, client_name, ""};
            used_names.insert(client_name);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    logMessage("Новый клиент подключен: " + client_name + " (сокет: " + to_string(new_socket) + ")");
                    cout << "[" << getCurrentTime() << "] " << "Новый клиент подключен: " << client_name << " (сокет: " << new_socket << ")" << endl;
                    sendToClient(new_socket, "\033[32mДобро пожаловать, " + client_name + "!\033[0m\nИспользуйте /help для списка команд.\n");
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    logMessage("Клиент отключился: " + clients[sd].name + " (сокет: " + to_string(sd) + ")");
                    cout << "[" << getCurrentTime() << "] " << "Клиент отключился: " << clients[sd].name << " (сокет: " << sd << ")" << endl;
                    if (!clients[sd].current_room.empty()) {
                        broadcastMessage(clients[sd].current_room, clients[sd].name + " вышел из чата.\n");
                        rooms[clients[sd].current_room].clients.erase(sd);
                    }
                    used_names.erase(clients[sd].name);
                    close(sd);
                    client_sockets[i] = 0;
                    clients.erase(sd);
                } else {
                    buffer[valread] = '\0';
                    string message(buffer);
                    message.erase(remove(message.begin(), message.end(), '\n'), message.end());
                    message.erase(remove(message.begin(), message.end(), '\r'), message.end());
                    handleCommand(sd, message);
                }
            }
        }
    }

    return 0;
}