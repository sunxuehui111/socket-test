#ifndef _EasyTcpSever_hpp_
#define _EasyTcpSever_hpp_
#define REC_BUFF_SIZE 10240

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<vector>
#include"MessagePacket.hpp"
using namespace std;
#pragma comment (lib,"ws2_32.lib")

class ClientSocket
{
public:
	ClientSocket(SOCKET sock = INVALID_SOCKET) {
		_socket = sock;
		_LastPostion = 0;
		memset(_recMesBuf, 0, 0);
	}
	SOCKET sockfd(){
		return _socket;
	}
	char* recMes() {
		return _recMesBuf;
	}
	int getLastPos() {
		return _LastPostion;
	}
	void setLastPos(int pos) {
		_LastPostion = pos;
	}
private:
	SOCKET _socket;
	//数据长度标志位
	int _LastPostion;
	//存放消息缓冲区
	char _recMesBuf[REC_BUFF_SIZE * 10];
};

class EasyTcpSever
{
	SOCKET _socket;
	std::vector<ClientSocket*> g_socklist;
public:
	EasyTcpSever() {
		_socket = INVALID_SOCKET;
	}
	virtual ~EasyTcpSever() {

	}

	//初始化socket 套接字
	bool initSocket() {

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		int error = 0;
		WSAStartup(version, &data);

		//关闭旧连接
		if (isRun()) {
			close();
		}
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (!isRun()) {
			return false;
		}
		return true;
	}
	//绑定套接字
	bool Bind(const char* ip, unsigned short port) {
		if (!isRun()) {
			initSocket();
		}
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if (ip == nullptr) {
			addr.sin_addr.S_un.S_addr = ADDR_ANY;
			addr.sin_addr.s_addr = ADDR_ANY;
		}
		else
		{
			addr.sin_addr.S_un.S_addr = inet_addr(ip);
			addr.sin_addr.s_addr = inet_addr(ip);
		}
		int error = bind(_socket, (sockaddr*)&addr, sizeof(addr));
		if (error == SOCKET_ERROR) {
			printf("绑定端口号失败!\n");
			return false;
		}
		else
		{
			printf("绑定socket<%d>成功!\n", _socket);
		}
		return true;
	}
	//监听套接字
	bool Listen(int n) {
		if (!isRun()) {
			return false;
		}
		int error = listen(_socket, n);
		if (error) {
			printf("监听socket<%d>失败!\n", _socket);
			return false;
		}
		else
		{
			printf("监听socket<%d>成功!\n", _socket);
		}
		return true;
	}
	bool Accept()
	{
		sockaddr_in clientAddr;
		int cLen = sizeof(sockaddr_in);
		SOCKET client = INVALID_SOCKET;
		client = accept(_socket, (sockaddr*)&clientAddr, &cLen);
		if (client == INVALID_SOCKET) {
			return false;
		}
		printf("有一个连接进来了！IP 为：%s \n", inet_ntoa(clientAddr.sin_addr));
		NewClientJoin joinmes;
		sendToAllData(&joinmes);
		g_socklist.push_back(new ClientSocket(client));
		return true;
	}
	//关闭
	void close() {
		if (isRun()) {
			for (int n = (int)g_socklist.size() - 1; n >= 0; n--)
			{
				closesocket(g_socklist[n]->sockfd());
				delete g_socklist[n];
			}
			WSACleanup();
		}
	}
	//网络套接字判断
	bool goRun() {
		if (!isRun()) {
			return false;
		}
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_exp;

		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_exp);

		FD_SET(_socket, &fd_read);
		FD_SET(_socket, &fd_write);
		FD_SET(_socket, &fd_exp);

		for (int n = (int)g_socklist.size() - 1; n >= 0; n--) {
			FD_SET(g_socklist[n]->sockfd(), &fd_read);
		}
		timeval time = { 1,0 };
		int ret = select(_socket + 1, &fd_read, &fd_write, &fd_exp, &time);
		if (ret < 0) {
			printf("select<%d>退出", _socket);
			close();
			return false;
		}
		if (FD_ISSET(_socket, &fd_read)) {
			FD_CLR(_socket, &fd_read);
			Accept();
		}
		//for (size_t n = 0; n < fd_read.fd_count; n++)
		//{
		//	if (!RecvData(fd_read.fd_array[n]))
		//	{
		//		auto iter = find(g_socklist.begin(), g_socklist.end(), fd_read.fd_array[n]);
		//		if (iter != g_socklist.end())
		//		{
		//			delete g_socklist[n];
		//			g_socklist.erase(iter);
		//		}
		//	}
		//}
		for (int n = (int)g_socklist.size() - 1; n >= 0; n--) {
			if (FD_ISSET(g_socklist[n]->sockfd(), &fd_read)) {
				if (false == RecvData(g_socklist[n])) {
					auto iter = g_socklist.begin() + n;
					if (iter != g_socklist.end())
					{
						delete g_socklist[n];
						g_socklist.erase(iter);
					}
				}
			}
		}
		return true;
	}
	char recvbuf[REC_BUFF_SIZE] = {};
	//接收数据
	bool  RecvData(ClientSocket* _Socket) {
		/*char recvbuf[1024];
		DataHeader* header = (DataHeader*)recvbuf;*/
		int nlen = recv(_Socket->sockfd(), recvbuf, REC_BUFF_SIZE, 0);
		if (nlen <= 0) {
			printf("客户端<%d>退出 \n", _Socket->sockfd());
			return false;
		}
		/*recv(_Socket, recvbuf + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
		RecvNetMesg(_Socket,header);*/
		//将接收到的数据拷贝到消息缓冲区
		memcpy(_Socket->recMes() + _Socket->getLastPos(), recvbuf, nlen);
		//将消息缓冲区的数据尾部后移
		_Socket->setLastPos(_Socket->getLastPos() + nlen);
		//判断消息缓冲区长度是否大于消息头大小
		while (_Socket->getLastPos() >= sizeof(DataHeader))
		{
			//当前消息长度
			DataHeader*  header = (DataHeader*)_Socket->recMes();
			//判断消息长度是否大于数据长度
			if (_Socket->getLastPos() >= header->datalength) {
				//消息缓冲区剩余未处理消息长度
				int nSize = _Socket->getLastPos() - header->datalength;
				//处理网络消息
				RecvNetMesg(_Socket->sockfd(),header);
				//将消息缓冲区未处理数据前移
				memcpy(_Socket->recMes(), _Socket->recMes() + header->datalength, nSize);
				//消息缓冲区尾部前移
				_Socket->setLastPos(nSize);
			}
			else
			{
				//消息长度不够 继续收发
				break;
			}
		}
		return true;
	}
	//发送数据
	int sendData(SOCKET client, DataHeader* header)
	{
		if (isRun() && header) {
			return send(client, (const char*)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}
	int sendToAllData(DataHeader* header)
	{
		for (int n = (int)g_socklist.size() - 1; n >= 0; n--)
		{
			return send(g_socklist[n]->sockfd(), (const char*)header, header->datalength, 0);
		}
	}
	//处理消息
	void RecvNetMesg(SOCKET client,DataHeader* header) {
		switch (header->cmd)
		{
			case CMD_Login:
			{
				Login* login = (Login*)header;
				printf("CMD = CMD_Login,length = %d,username = %s,password = %s \n", login->datalength, login->UserName, login->Password);
				LoginResult loginreult;
				sendData(client, &loginreult);
			}break;
			case CMD_LoginOut:
			{
				LoginOut* loginout = (LoginOut*)header;;
				printf("CMD = CMD_LoginOutlength = %d,username = %s \n", loginout->datalength, loginout->UserName);
				LoginOutResult loginoutreult;
				sendData(client, &loginoutreult);
			}break;
			default:
			{
				DataHeader resdh = { 0,CMD_Error };
				sendData(client, &resdh);
			}
			break;
		}
	}
	//判断是否在运行
	bool isRun() {
		return _socket != INVALID_SOCKET;
	}
private:

};
#endif // !_EasyTcpSever_hpp_
