#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "netbus/session.h"

class Service
{
public:
	bool using_raw;
	Service();
public:
	//bool��ʾ �յ���ʱΥ��������ɵ��������	false��ʾ��Υ������
	virtual bool recvServiceCmd(Session* s, commandMessage* msg);
	//�ر����Ӳ�ͬ��������ģ��
	virtual bool disconnectService(Session* s , int sType);

	virtual bool recvServiceRawHead(Session* s, struct Gateway2Server* msg);
};

#endif