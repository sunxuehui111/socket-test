
#include"EasyTcpSever.hpp"

int main(void) {
	EasyTcpSever sever;
	sever.initSocket();
	sever.Bind(nullptr, 8887);
	sever.Listen(5);

	while (sever.isRun()) {
		sever.goRun();
	}
	sever.close();
	return 0;
}
