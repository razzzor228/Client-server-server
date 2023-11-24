#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class ResultDisplayServer {
public:
    ResultDisplayServer(int listeningPort) : listeningPort_(listeningPort) {}

    void Start() {
        WSADATA wsData;
        if (WSAStartup(MAKEWORD(2, 2), &wsData) != NO_ERROR) {
            std::cerr << "Failed to initialize Winsock" << std::endl;
            return;
        }

        // Создаем сокет для прослушивания входящих соединений
        SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << "Failed to create listen socket" << std::endl;
            WSACleanup();
            return;
        }

        // Подготовка адреса прослушивания
        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(listeningPort_);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        // Привязка сокета к адресу прослушивания
        if (bind(listenSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
            std::cerr << "Failed to bind listen socket" << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return;
        }

        // Переводим сокет в режим прослушивания
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Failed to listen on listen socket" << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return;
        }

        std::cout << "Server is listening for incoming connections" << std::endl;

        while (true) {
            // Ожидаем входящего соединения
            SOCKET clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Failed to accept client connection" << std::endl;
                closesocket(listenSocket);
                WSACleanup();
                return;
            }

            std::cout << "Client connected" << std::endl;

            std::string receivedData;
            char buffer[4096];

            // Получаем данные от клиента обработки данных и выводим их на экран
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (bytesRead <= 0) {
                    std::cerr << "Connection closed by client" << std::endl;
                    break;
                }
                receivedData += buffer;
                std::cout << "Received data: " << receivedData << std::endl;
            }

            closesocket(clientSocket);
            std::cout << "Client disconnected" << std::endl;
        }

        closesocket(listenSocket);
        WSACleanup();
    }

private:
    int listeningPort_;
};

int main(int argc, char* argv[]) {
    int port;

    if (argc == 2) {
        port = std::stoi(argv[1]);
    }
    else {
        std::cout << "Enter listening port: ";
        std::cin >> port;
    }

    ResultDisplayServer server(port);
    server.Start();
    _CrtDumpMemoryLeaks();
    return 0;
}//2048 - пример входных данных в консоль или в строку аргументы команд