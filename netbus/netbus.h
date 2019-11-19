#ifndef __NETBUS_H__
#define __NETBUS_H__

class Session;

class NetBus {
public:

	//ȫ��Ψһʵ��
	static NetBus* instance();

public:
	//��������ʼ��װ��
	void init();

	//����������
	void run();

	//tcp���ӿ���
	void startTcpServer(const char* ip, int port);

	//web���ӿ���
	void startWebServer(const char* ip, int port);

	//udp����
	void startUdpServer(const char* ip, int port);

	void tcp_connect(const char* ip, int port, void(*on_connected)(int err, Session* s, void* udata), void* udata);

};

#endif