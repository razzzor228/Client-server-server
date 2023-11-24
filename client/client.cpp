#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class DataProcessingClient {
public:
    DataProcessingClient(const std::string& serverAddress, int serverPort) :
        serverAddress_(serverAddress), serverPort_(serverPort) {}

    void Start() {
        WSADATA wsData;
        if (WSAStartup(MAKEWORD(2, 2), &wsData) != NO_ERROR) {
            std::cerr << "Failed to initialize Winsock" << std::endl;
            return;
        }

        // Создаем сокет для подключения к серверу обработки данных
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to create client socket" << std::endl;
            WSACleanup();
            return;
        }

        // Подготовка адреса сервера обработки данных
        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(serverPort_);
        inet_pton(AF_INET, serverAddress_.c_str(), &(serverAddress.sin_addr));

        // Подключение к серверу обработки данных
        if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
            std::cerr << "Failed to connect to server" << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return;
        }

        std::cout << "Connected to server" << std::endl;

        while (true) {
            std::string inputData;
            std::cout << "Enter data to send to the server (press Enter to send): ";
            std::getline(std::cin, inputData);

            if (inputData.empty()) {
                std::cout << "Nothing entered. Exiting..." << std::endl;
                break;
            }

            // Отправка данных на сервер обработки данных
            if (send(clientSocket, inputData.c_str(), inputData.length(), 0) == SOCKET_ERROR) {
                std::cerr << "Failed to send data to server" << std::endl;
                break;
            }

            std::cout << "Data sent to server" << std::endl;
        }

        // Закрытие сокета и очистка ресурсов
        closesocket(clientSocket);
        WSACleanup();
    }

private:
    std::string serverAddress_;
    int serverPort_;
};

int main(int argc, char* argv[]) {
    std::string serverAddress;
    int serverPort;

    // Проверка количества аргументов командной строки
    if (argc < 3) {
        std::cout << "Enter Server Address: ";
        std::cin >> serverAddress;

        std::cout << "Enter Server Port: ";
        std::cin >> serverPort;
        std::cin.ignore();// Эта строчка нужна для того, чтобы не игнорировать getline()
    }
    else {
        serverAddress = argv[1];
        serverPort = std::stoi(argv[2]);
    }

    DataProcessingClient client(serverAddress, serverPort);
    client.Start();

    return 0;
}//127.0.0.1 12345 - пример входных данных в консоль или в строку аргументы команд