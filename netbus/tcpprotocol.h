#ifndef __TCPPROTOCOL_H__
#define __TCPPROTOCOL_H__

class tcpProtocol
{
public:
	//��ȡЭ��ͷ���� 
	static bool readTcpHeader(unsigned char* data,int dataLen,int* pkgSize,int* outHeaderSize);
	//����������������
	static unsigned char* parserRecvData(const unsigned char* rawData,int len,int* pkgLen);
	//���ض�Ӧ�Ĳ���
	static void freePackageData(unsigned char* tcpPkg);
};

#endif