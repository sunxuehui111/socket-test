#include"EasyTcpClient.hpp"
#include<thread>


void scanfThread(EasyTcpClient* client)
{
	while (true) {
		char sendbuf[128] = {};
		std::cin >> sendbuf;
		//scanf_s("%s",sendbuf,128);
		//sendbuf[scanfLen] = '\0';
		if (0 == strcmp(sendbuf, "exit")) {
			client->close();
			break;
		}
		else if (0 == strcmp(sendbuf, "login")) {
			Login login;
			strcpy_s(login.UserName, "tom");
			strcpy_s(login.Password, "tom mm");
			client->sendData(&login);
		}
		else if (0 == strcmp(sendbuf, "loginout")) {
			LoginOut loginout;
			strcpy_s(loginout.UserName, "tom");
			client->sendData(&loginout);
		}
		else {
			printf("error cmd,input again \n");
		}
	}
}
int main(void) {
	EasyTcpClient client;
	client.ConnectSock("127.0.0.1", 8887);
	std::thread t1(scanfThread, &client);
	while (client.isRun())
	{
		client.goRun();
	}
	client.close();
	getchar();
	return 0;
}

