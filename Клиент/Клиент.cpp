#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32.lib")
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <locale>
#include <iostream>
#include <string>

using namespace std;

#define PORT 5000
#define BUF_SIZE 1024
#define SERVER_ADDR "127.0.0.1"
#define START_GAME_PHRASE "THE GAME STARTS"

WSAData wsaData;
SOCKET my_sock;

void closeServer();
bool checkSocketError(int);
int play();

int main() {
	setlocale(LC_ALL, "Rus");
	printf("Клиент из стартед!\n");
	if (WSAStartup(MAKEWORD(2, 2), (WSADATA*)&wsaData))
	{
		printf("WSAStart ошибка %i\n", WSAGetLastError());
		system("pause");
		return -1;
	}
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock < 0)
	{
		printf("Socket() ошибка %i\n", WSAGetLastError());
		system("pause");
		return -1;
	}
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT* hst;
	if (inet_addr(SERVER_ADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	else
		if (hst = gethostbyname(SERVER_ADDR))
			((unsigned long*)&dest_addr.sin_addr)[0] =
			((unsigned long**)hst->h_addr_list)[0][0];
		else
		{
			printf("Инвалид адрес %s\n", SERVER_ADDR);
			closeServer();
			return -1;
		}
	if (connect(my_sock, (sockaddr*)&dest_addr, sizeof(dest_addr)))
	{
		printf("Коннект эррор %i\n", WSAGetLastError());
		system("pause");
		return -1;
	}
	printf("Соединение с %s успешно установлено\n\n", SERVER_ADDR);

	int bytesReceived;
	//int bytesSent;
	char buf[BUF_SIZE] = "\0";
	string myAnswer;
	ZeroMemory(buf, BUF_SIZE);
	// Получаем приветствие
	bytesReceived = recv(my_sock, &buf[0], BUF_SIZE, 0);
	if (checkSocketError(bytesReceived)) return -1;
	cout << buf << endl;
	send(my_sock, "received successfully", 22, 0);
	// Получаем порт
	ZeroMemory(buf, BUF_SIZE);
	bytesReceived = recv(my_sock, &buf[0], BUF_SIZE, 0);
	if (checkSocketError(bytesReceived)) return -1;
	if (string(buf).find("Your id") != -1) {
		string portFromServerStr = string(buf);
		int ourId = std::atoi(portFromServerStr.substr(8, portFromServerStr.length() - 1).c_str());
		cout << "Your id: " << ourId << endl;
	}
	else {
		cout << "id не получен!" << endl;
	}

	string keys[] = { "Игра уже началась", "Ты добавлен в очередь" };
	ZeroMemory(buf, BUF_SIZE);
	bytesReceived = recv(my_sock, &buf[0], BUF_SIZE, 0);
	if (checkSocketError(bytesReceived)) return -1;
	cout << "\n" << buf << endl;

	//if (string(buf).find(keys[0]) != -1) {
	//	// Покидаем сервер
	//	return -1;
	//}

	if (string(buf).find(keys[1]) != -1) {
		// Сидим в очереди

		while (strcmp(buf, START_GAME_PHRASE) != 0) {
			ZeroMemory(buf, BUF_SIZE);
			bytesReceived = recv(my_sock, &buf[0], BUF_SIZE, 0);
			if (checkSocketError(bytesReceived)) return -1;
			cout << buf << endl << endl;
		}
		if (play() == -1) return 1;
	}

	closeServer();
	return 0;
}

void closeServer() {
	closesocket(my_sock);
	WSACleanup();
	cout << "Закрытие связи с сервером." << endl;
	system("pause");
}

bool checkSocketError(int bytes) {
	if (bytes == SOCKET_ERROR || bytes == 0) {
		std::cerr << "Ошибка в приёме сообщения." << std::endl;
		closeServer();
		return true;
	}
	return false;
}

int play() {
	char buf[1024]{};
	int bytesReceived;
	int bytesSent;
	string myAnswer;
	printf("Игра начинается\n");
	string keys[] = { "проиграл", "выиграл" };
	bool finish = false;
	while (true) {
		ZeroMemory(buf, BUF_SIZE);
		while (true) {
			/*ZeroMemory(buf, BUF_SIZE);
			recv(my_sock, &buf[0], BUF_SIZE, 0);
			send(my_sock, "hgvj" , 5, 0);
			cout << buf << endl;*/
			ZeroMemory(buf, BUF_SIZE);
			bytesReceived = recv(my_sock, &buf[0], BUF_SIZE, 0);
			if (checkSocketError(bytesReceived)) return -1;
			cout << buf << endl;
			if (string(buf).find(keys[0]) != -1 || string(buf).find(keys[1]) != -1) {
				finish = true;
				break;
			}
			if (string(buf).find("ход переходит") != -1) break;
			cout << "Введите ваш ответ: ";
			std::getline(std::cin, myAnswer);
			bytesSent = send(my_sock, myAnswer.c_str(), myAnswer.length(), 0);
		}
		if (finish) break;
	}
	if (string(buf).find(keys[0]) != -1) {
		// проигрыш
	}
	if (string(buf).find(keys[1]) != -1) {
		// победа 
	}
}