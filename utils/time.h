#ifndef __TIME_H__
#define __TIME_H__

#include <fcntl.h>
#include <time.h>
#include <string>
#include "uv.h"

static uint32_t gCurrrentDay;
static uint32_t gLastSecond;
static char gFormatTime[64] = { 0 };

struct timer
{
	uv_timer_t uvTimer;
	void(*onTimer)(void* uData);
	void* uData;
	//-1 == һֱѭ��  
	int repeatCount;
};

#ifdef __cplusplus
extern "C" {
#endif // _cplusplus

	//����һ����ʱ��
	struct timer* scheduleRepeat(
		//��ʱ��������ʱ����õĺ���		data���û��Զ������
		void(*onTimer)(void* uData),
		void* uData,
		//�������ʼִ�������ʱ��
		int afterMsec,
		//ִ�ж��ٴ����������ʱ��   -1��ʾһֱִ��
		int repeatCount,
		//ÿִ��һ��֮�� ��������ٴ�ִ��
		int repeatmsec
	);

	//�ͷ�һ����ʱ��
	void cancelTimer(timer* t);

	//����һ�����μ�ʱ��
	struct timer* scheduleOnce(
		void(*onTimer)(void* uData),
		void* uData,
		int afterMsec
	);
	void* get_timer_udata(struct timer* t);
	//�õ���ǰʱ���
	unsigned long timeStamp();

	//����ת��ʱ���
	unsigned long dateToTimeStamp(const char* formatDate, const char* date);

	//ʱ���ת����
	void timeStampToDate(unsigned long t, const char* formatDate,char* outBuf,int buffLen);

	//��Ҫ�����ʱ����Լ�ת
	unsigned long timeStampToday();

#ifdef __cplusplus
}
#endif // _cplusplus

#endif