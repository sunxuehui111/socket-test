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
	//���ݳ��ȱ�־λ
	int _LastPostion;
	//�����Ϣ������
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
	//���׽���
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
			printf("�󶨶˿ں�ʧ��!\n");
			return false;
		}
		else
		{
			printf("��socket<%d>�ɹ�!\n", _socket);
		}
		return true;
	}
	//�����׽���
	bool Listen(int n) {
		if (!isRun()) {
			return false;
		}
		int error = listen(_socket, n);
		if (error) {
			printf("����socket<%d>ʧ��!\n", _socket);
			return false;
		}
		else
		{
			printf("����socket<%d>�ɹ�!\n", _socket);
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
		printf("��һ�����ӽ����ˣ�IP Ϊ��%s \n", inet_ntoa(clientAddr.sin_addr));
		NewClientJoin joinmes;
		sendToAllData(&joinmes);
		g_socklist.push_back(new ClientSocket(client));
		return true;
	}
	//�ر�
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
	//�����׽����ж�
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
			printf("select<%d>�˳�", _socket);
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
	//��������
	bool  RecvData(ClientSocket* _Socket) {
		/*char recvbuf[1024];
		DataHeader* header = (DataHeader*)recvbuf;*/
		int nlen = recv(_Socket->sockfd(), recvbuf, REC_BUFF_SIZE, 0);
		if (nlen <= 0) {
			printf("�ͻ���<%d>�˳� \n", _Socket->sockfd());
			return false;
		}
		/*recv(_Socket, recvbuf + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
		RecvNetMesg(_Socket,header);*/
		//�����յ������ݿ�������Ϣ������
		memcpy(_Socket->recMes() + _Socket->getLastPos(), recvbuf, nlen);
		//����Ϣ������������β������
		_Socket->setLastPos(_Socket->getLastPos() + nlen);
		//�ж���Ϣ�����������Ƿ������Ϣͷ��С
		while (_Socket->getLastPos() >= sizeof(DataHeader))
		{
			//��ǰ��Ϣ����
			DataHeader*  header = (DataHeader*)_Socket->recMes();
			//�ж���Ϣ�����Ƿ�������ݳ���
			if (_Socket->getLastPos() >= header->datalength) {
				//��Ϣ������ʣ��δ������Ϣ����
				int nSize = _Socket->getLastPos() - header->datalength;
				//����������Ϣ
				RecvNetMesg(_Socket->sockfd(),header);
				//����Ϣ������δ��������ǰ��
				memcpy(_Socket->recMes(), _Socket->recMes() + header->datalength, nSize);
				//��Ϣ������β��ǰ��
				_Socket->setLastPos(nSize);
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
	//������Ϣ
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
	//�ж��Ƿ�������
	bool isRun() {
		return _socket != INVALID_SOCKET;
	}
private:

};
#endif // !_EasyTcpSever_hpp_
