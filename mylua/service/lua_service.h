#ifndef __LUA_SERVICE_H__
#define __LUA_SERVICE_H__

#include "netbus/service.h"

int regist_service_export(lua_State* L);
int regist_releaseRaw_export(lua_State* L);

class lua_service :public Service
{
public:
	unsigned int luaRecvCmdHandle;
	unsigned int luaDisconHandle;
	unsigned int luaRecvRawHandle;

public:
	//bool��ʾ �յ���ʱΥ��������ɵ��������	false��ʾ��Υ������
	virtual bool recvServiceCmd(Session* s, commandMessage* msg);

	virtual bool recvServiceRawHead(Session* s, Gateway2Server* msg);
	//�ر����Ӳ�ͬ��������ģ��
	virtual bool disconnectService(Session* s, int sType);
};
#endif // !__LUA_SERVICE_H__