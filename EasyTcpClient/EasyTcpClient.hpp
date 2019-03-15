#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include"MessagePacket.hpp"
#pragma comment (lib,"ws2_32.lib")
#define REC_BUFF_SIZE 10240
using namespace std;
class EasyTcpClient
{
	SOCKET _socket;
public:
	
	EasyTcpClient() {
		_socket = INVALID_SOCKET;
	}

	virtual ~EasyTcpClient() {

		close();
	}

public:
	//数据长度标志位
	int _LastPostion;
	//接收缓冲区大小
	char _recbuf[REC_BUFF_SIZE];
	//存放消息缓冲区
	char _recMesBuf[REC_BUFF_SIZE * 10];
public:
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
	//连接服务器
	bool ConnectSock(const char* ip, unsigned short port) {
		if (!isRun()) {
			initSocket();
		}
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = inet_addr(ip);
		int error = connect(_socket, (sockaddr*)&addr, sizeof(addr));
		if (error == SOCKET_ERROR) {
			return false;
		}
		else
		{
			printf("连接socket<%s>成功！", ip);
		}
		return true;
	}
	//关闭
	void close() {
		if (isRun()) {
			closesocket(_socket);
			WSACleanup();
		}
	}
	//网络套接字判断
	bool goRun() {
		if (!isRun()) {
			return false;
		}
		fd_set fd_read;
		FD_ZERO(&fd_read);
		FD_SET(_socket, &fd_read);
		timeval time = { 1,0 };
		int ret = select(_socket, &fd_read, 0, 0, &time);
		if (ret < 0) {
			printf("select退出");
			return false;
		}
		if (FD_ISSET(_socket, &fd_read)) {
			FD_CLR(_socket, &fd_read);
			if (false == RecvData()) {
				printf("与服务器断开连接");
			}
		}
		return true;
	}
	//接收数据
	bool  RecvData() {
		int nlen = recv(_socket, _recbuf, REC_BUFF_SIZE, 0);
		if (nlen <= 0) {
			return false;
		}
		//将接收到的数据拷贝到消息缓冲区
		memcpy(_recMesBuf + _LastPostion, _recbuf, nlen);
		//将消息缓冲区的数据尾部后移
		_LastPostion += nlen;
		//判断消息缓冲区长度是否大于消息头大小
		while (_LastPostion >= sizeof(DataHeader))
		{
			//当前消息长度
			DataHeader*  header = (DataHeader*)_recMesBuf;
			//判断消息长度是否大于数据长度
			if (_LastPostion >= header->datalength) {
				//消息缓冲区剩余未处理消息长度
				int nSize = _LastPostion - header->datalength;
				//处理网络消息
				RecvNetMesg(header);
				//将消息缓冲区未处理数据前移
				memcpy(_recMesBuf, _recMesBuf + header->datalength, nSize);
				//消息缓冲区尾部前移
				_LastPostion = nSize;
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
	int sendData(DataHeader* header)
	{
		if (isRun() && header) {
			return send(_socket, (const char*)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}
	//处理消息
	void RecvNetMesg(DataHeader* header) {
		switch (header->cmd)
		{
			case CMD_Login_Result:
			{
				LoginResult* loginResult = (LoginResult*)header;
				printf("CMD = CMD_Login_Result,datalength = %d \n", loginResult->datalength);
			}break;
			case CMD_LoginOut_Result:
			{
				LoginOutResult* loginoutresult = (LoginOutResult*)header;
				printf("CMD = CMD_LoginOut_Result,datalength = %d \n", loginoutresult->datalength);
			}break;
			case CMD_NewClient_Join:
			{
				NewClientJoin* newClientJoin = (NewClientJoin*)header;
				printf("CMD = CMD_NewClient_Join,datalength = %d \n", newClientJoin->datalength);
			}break;
			case CMD_Error:
			{
				printf("CMD = CMD_Error,datalength = %d \n", header->datalength);
			}break;
			default:
			{
				printf("收到未知消息,datalength = %d \n", header->datalength);
			}
		}
	}
	//判断是否在运行
	bool isRun() {
		return _socket != INVALID_SOCKET;
	}

private:

};

#endif