#ifndef __WEBPROTOCOL_H__
#define __WEBPROTOCOL_H__

class webProtocol
{
public:
	//��ͻ�������
	static bool webserShakeHand(Session* s, char* body, int len);
	//��ȡЭ��ͷ �ֳ�head���ݺ�body����
	static bool readWebHeader(unsigned char* pkgData, int pkgLen, int* pkgSize,int* outHeaderSize);
	//��ԭ���ܵ�http��������
	static void parserRecvData(unsigned char* rawData, unsigned char* mask,int rawLen);
	//���͸��ͻ��˵����ݰ�
	static unsigned char* sendPackageData(const unsigned char* raw_data, int len,int* webDataLen);
	//����ѷ��͵İ�������
	static void freePackageData(unsigned char* ws_pkg);
};

#endif