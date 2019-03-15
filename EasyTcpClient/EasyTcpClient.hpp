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
	//���ݳ��ȱ�־λ
	int _LastPostion;
	//���ջ�������С
	char _recbuf[REC_BUFF_SIZE];
	//�����Ϣ������
	char _recMesBuf[REC_BUFF_SIZE * 10];
public:
	//��ʼ��socket �׽���
	bool initSocket() {

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		int error = 0;
		WSAStartup(version, &data);

		//�رվ�����
		if (isRun()) {
			close();
		}
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (!isRun()) {
			return false;
		}
		return true;
	}
	//���ӷ�����
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
			printf("����socket<%s>�ɹ���", ip);
		}
		return true;
	}
	//�ر�
	void close() {
		if (isRun()) {
			closesocket(_socket);
			WSACleanup();
		}
	}
	//�����׽����ж�
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
			printf("select�˳�");
			return false;
		}
		if (FD_ISSET(_socket, &fd_read)) {
			FD_CLR(_socket, &fd_read);
			if (false == RecvData()) {
				printf("��������Ͽ�����");
			}
		}
		return true;
	}
	//��������
	bool  RecvData() {
		int nlen = recv(_socket, _recbuf, REC_BUFF_SIZE, 0);
		if (nlen <= 0) {
			return false;
		}
		//�����յ������ݿ�������Ϣ������
		memcpy(_recMesBuf + _LastPostion, _recbuf, nlen);
		//����Ϣ������������β������
		_LastPostion += nlen;
		//�ж���Ϣ�����������Ƿ������Ϣͷ��С
		while (_LastPostion >= sizeof(DataHeader))
		{
			//��ǰ��Ϣ����
			DataHeader*  header = (DataHeader*)_recMesBuf;
			//�ж���Ϣ�����Ƿ�������ݳ���
			if (_LastPostion >= header->datalength) {
				//��Ϣ������ʣ��δ������Ϣ����
				int nSize = _LastPostion - header->datalength;
				//����������Ϣ
				RecvNetMesg(header);
				//����Ϣ������δ��������ǰ��
				memcpy(_recMesBuf, _recMesBuf + header->datalength, nSize);
				//��Ϣ������β��ǰ��
				_LastPostion = nSize;
			}
			else
			{
				//��Ϣ���Ȳ��� �����շ�
				break;
			}
		}
		return true;
	}
	//��������
	int sendData(DataHeader* header)
	{
		if (isRun() && header) {
			return send(_socket, (const char*)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}
	//������Ϣ
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
				printf("�յ�δ֪��Ϣ,datalength = %d \n", header->datalength);
			}
		}
	}
	//�ж��Ƿ�������
	bool isRun() {
		return _socket != INVALID_SOCKET;
	}

private:

};

#endif