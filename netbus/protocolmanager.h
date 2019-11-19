#ifndef __PROTOCOLMANAGER_H__
#define __PROTOCOLMANAGER_H__
#include "google/protobuf/message.h"
#include <string>
#include <map>

using namespace google::protobuf;
//Э�����
enum
{
	PROTOCOL_JSON = 0,
	PROTOCOL_BUF = 1,
};

//�ͻ�������
struct commandMessage 
{
	//�����
	int serverType; 
	//�����
	int commandType;
	//�û�
	unsigned int tags;
	//����
	void* body;
};

struct Gateway2Server
{
	//�����
	int serverType;
	//�����
	int commandType;
	//�û�
	unsigned int tags;

	unsigned char* rawCmd;

	int rawLen;
};

class protocolManager
{
public:
	//��ʼ��Э����Ϣ
	static void init(int protocolType);
	//�鿴Э������
	static int getProtocolType();
	//�õ���ǰ����������
	static const char* getProtoCurrentMapName(int commandType);
	//����һ��Э����Ϣ
	static Message* createMessage(const char* nameType);
	//ɾ��Э����Ϣ
	static void releaseMessage(Message* message);
	//ע��Э��ӳ���
	static void registerCmdMap(std::map<int,std::string>& protoMap);
	//����Ҫ���͵�����
	static bool decodeCmdMsg(unsigned char* cmd,int cmdLen,struct commandMessage** outMessage);
	static void freeDecodeCmdMsg(struct commandMessage* cmdmsg);
	//���ܷ�����������
	static unsigned char* encodeCmdMsg(const struct commandMessage* cmdmsg,int* outLen);
	static void freeEncodeCmdMsg(unsigned char* raw);
	//���ؽ���ͷ��ת������
	static bool decodeMsgHead(unsigned char* cmd, int cmdLen, struct Gateway2Server* outRaw);
};

#endif