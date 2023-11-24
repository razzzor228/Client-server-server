#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class DataProcessingServer {
public:
    DataProcessingServer(const std::string& displayServerAddress, int displayServerPort, int listeningPort) :
        displayServerAddress_(displayServerAddress), displayServerPort_(displayServerPort), listeningPort_(listeningPort) {}

    void Start() {
        WSADATA wsData;
        if (WSAStartup(MAKEWORD(2, 2), &wsData) != NO_ERROR) {
            std::cerr << "Failed to initialize Winsock" << std::endl;
            return;
        }

        // Создаем сокет для прослушивания подключений
        SOCKET listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listeningSocket == INVALID_SOCKET) {
            std::cerr << "Failed to create listening socket" << std::endl;
            WSACleanup();
            return;
        }

        // Подготовка адреса сервера обработки данных
        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(listeningPort_);

        // Привязываем сокет к адресу сервера
        if (bind(listeningSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
            std::cerr << "Failed to bind listening socket" << std::endl;
            closesocket(listeningSocket);
            WSACleanup();
            return;
        }

        // Ожидаем подключения клиента
        if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Failed to start listening for connections" << std::endl;
            closesocket(listeningSocket);
            WSACleanup();
            return;
        }

        std::cout << "Data Processing Server started, waiting for connections..." << std::endl;

        // Принимаем входящие подключения
        SOCKET clientSocket = accept(listeningSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept incoming connection" << std::endl;
            closesocket(listeningSocket);
            WSACleanup();
            return;
        }

        std::cout << "Client connected" << std::endl;

        // Подключение к серверу отображения результата
        SOCKET displaySocket = ConnectToDisplayServer();
        if (displaySocket == INVALID_SOCKET) {
            std::cerr << "Failed to connect to display server" << std::endl;
            closesocket(clientSocket);
            closesocket(listeningSocket);
            WSACleanup();
            return;
        }
        
        // Получаем данные от клиента и обрабатываем их
        std::string data = ReceiveData(clientSocket);
        std::string processedData = ProcessData(data);

        // Отправляем подтверждение клиенту
        SendData(clientSocket, "Data received");

        // Передаем результат на сервер отображения результата
        SendData(displaySocket, processedData);

        // Закрываем сокеты
        closesocket(clientSocket);
        closesocket(displaySocket);
        closesocket(listeningSocket);
        WSACleanup();
    }

private:
    std::string displayServerAddress_;
    int displayServerPort_;

    int listeningPort_;

    SOCKET ConnectToDisplayServer() {
        SOCKET displaySocket = socket(AF_INET, SOCK_STREAM, 0);
        if (displaySocket == INVALID_SOCKET) {
            std::cerr << "Failed to create display server socket" << std::endl;
            return INVALID_SOCKET;
        }

        sockaddr_in displayServerAddress{};
        displayServerAddress.sin_family = AF_INET;

        // Преобразование адреса сервера отображения результата в формат sockaddr_in
        if (inet_pton(AF_INET, displayServerAddress_.c_str(), &(displayServerAddress.sin_addr)) != 1) {
            std::cerr << "Invalid display server address" << std::endl;
            closesocket(displaySocket);
            return INVALID_SOCKET;
        }

        displayServerAddress.sin_port = htons(displayServerPort_);

        // Подключение к серверу отображения результата
        if (connect(displaySocket, (sockaddr*)&displayServerAddress, sizeof(displayServerAddress)) == SOCKET_ERROR) {
            std::cerr << "Failed to connect to display server" << std::endl;
            closesocket(displaySocket);
            return INVALID_SOCKET;
        }

        std::cout << "Connected to Display Server" << std::endl;

        return displaySocket;
    }

    std::string ReceiveData(SOCKET socket) {
        std::string receivedData;
        char buffer[1024];

        int bytesRead;
        do {
            bytesRead = recv(socket, buffer, sizeof(buffer), 0);
            if (bytesRead > 0) {
                receivedData.append(buffer, bytesRead);
            }
        } while (bytesRead == sizeof(buffer));

        return receivedData;
    }

    void SendData(SOCKET socket, const std::string& data) {
        send(socket, data.c_str(), static_cast<int>(data.length()), 0);
    }

    std::string ProcessData(const std::string& data) {
        std::vector<std::string> words;

        // Преобразование данных в строку слов
        std::string word;
        for (char c : data) {
            if (c == ' ') {
                if (!word.empty()) {
                    words.push_back(word);
                    word.clear();
                }
            }
            else {
                word += c;
            }
        }

        if (!word.empty()) {
            words.push_back(word);
        }

        // Удаление дубликатов слов
        std::sort(words.begin(), words.end());
        words.erase(std::unique(words.begin(), words.end()), words.end());

        // Преобразование результата в строку
        std::string processedData;
        for (const std::string& word : words) {
            processedData += word + " ";
        }

        return processedData;
    }
};

int main(int argc, char* argv[]) {
    int listeningPort;
    std::string displayServerAddress;
    int displayServerPort;


    if (argc != 4) {
        std::cerr << "Usage: DataProcessingServer <ListeningPort> <displayServerAddress> <displayServerPort>" << std::endl;

        // Запрос ввода значений аргументов командной строки через консоль
        std::cout << "Enter Listening Port: ";
        std::cin >> listeningPort;

        std::cout << "Enter Display Server Address: ";
        std::cin >> displayServerAddress;

        std::cout << "Enter Display Server Port: ";
        std::cin >> displayServerPort;
    }
    else {
        listeningPort = std::stoi(argv[1]);
        displayServerAddress = argv[2];
        displayServerPort = std::stoi(argv[3]);
    }

    DataProcessingServer server(displayServerAddress, displayServerPort, listeningPort);
    server.Start();
    
    return 0;
}//12345 127.0.0.1 2048- пример входных данных в консоль или в строку аргументы команд