#ifndef __SERVICEMANAGER_H__
#define __SERVICEMANAGER_H__

#include "netbus/service.h"

class ServiceManager
{
public:
	//��ʼ������
	static void init();
	//ע�����   tpye ��user , system , default. 
	static bool registService(int type, Service* service);
	//ִ�з����е�����
	static bool recvCmdMsg(Session* session, Gateway2Server* msg);
	//�ͷŷ���
	static void sessionDisconnect(Session* session);
};

#endif