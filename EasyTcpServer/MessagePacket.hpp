#ifndef _MessagePacket_hpp_
#define _MessagePacket_hpp_

enum CMD
{
	CMD_Login,
	CMD_Login_Result,
	CMD_LoginOut,
	CMD_LoginOut_Result,
	CMD_NewClient_Join,
	CMD_Error
};

struct DataHeader
{
	short cmd;
	short datalength;
};

struct Login :public DataHeader
{
	Login() {
		cmd = CMD_Login;
		datalength = sizeof(Login);
	}
	char UserName[32];
	char Password[32];
};


struct  LoginResult :public DataHeader
{
	LoginResult() {
		cmd = CMD_Login_Result;
		datalength = sizeof(LoginResult);
		result = 0;
	}
	int result;
};

struct LoginOut :public DataHeader
{
	LoginOut() {
		cmd = CMD_LoginOut;
		datalength = sizeof(LoginOut);
	}
	char UserName[32];
};


struct  LoginOutResult :public DataHeader
{
	LoginOutResult() {
		cmd = CMD_LoginOut_Result;
		datalength = sizeof(LoginOutResult);
		result = 0;
	}
	int result;
};

struct  NewClientJoin :public DataHeader
{
	NewClientJoin() {
		cmd = CMD_NewClient_Join;
		datalength = sizeof(NewClientJoin);
		sockNum = 0;
	}
	int sockNum;
};
#endif // !_MessagePacket_hpp_