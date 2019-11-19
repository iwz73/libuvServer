#include "logger.h"

#include <string>
#include "time.h"
//ȫ�ֶ���
static std::string logPath;
static std::string logPrefix;
static bool log_stdOutPut=true;
static char* gLogLevel[] = { (char*)"DEBUG ", (char*)"WARNING ", (char*)"ERROR " };
//uv_fs�ľ��
static uv_fs_t gFileHandle;

static void openLogFile(tm* timeStruct)
{
	int result = 0;
	char fileName[128] = { 0 };
	//����ļ��Ѵ�
	if (gFileHandle.result != 0)
	{
		uv_fs_close(uv_default_loop(), &gFileHandle, gFileHandle.result, NULL);
		uv_fs_req_cleanup(&gFileHandle);
		gFileHandle.result = 0;
	}
	sprintf(fileName, "%s%s.%4d%02d%02d.log", logPath.c_str(), logPrefix.c_str(), timeStruct->tm_year + 1900, timeStruct->tm_mon + 1, timeStruct->tm_mday);
	result = uv_fs_open(NULL, &gFileHandle, fileName, O_CREAT | O_APPEND | O_RDWR, S_IREAD | S_IWRITE, NULL);
	if (result < 0)
	{
		fprintf(stderr, "open file failed! name=%s,reason=%s", fileName, uv_strerror(result));
	}
}

static void prepareFile()
{
	time_t now = time(NULL);
	now += 8 * 60 * 60;
	tm* tmStruct = gmtime(&now);
	//�����ͬһ�����־
	if (gFileHandle.result == 0)
	{
		gCurrrentDay = tmStruct->tm_mday;
		openLogFile(tmStruct);
	}
	else
	{
		if (gCurrrentDay != tmStruct->tm_mday)
		{
			gCurrrentDay = tmStruct->tm_mday;
			openLogFile(tmStruct);
		}
	}
}

static void format_time()
{
	time_t  now = time(NULL);
	now += 8 * 60 * 60;
	tm* time_struct = gmtime(&now);
	//��ӡ��ʱ����������
	if (now != gLastSecond) {
		gLastSecond = (uint32_t)now;
		memset(gFormatTime, 0, sizeof(gFormatTime));
		sprintf(gFormatTime, "%4d%02d%02d %02d:%02d:%02d ",
			time_struct->tm_year + 1900, time_struct->tm_mon + 1, time_struct->tm_mday,
			time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
	}
}

void Logger::init(const char* path, const char* prefix, bool std_output)
{
	//ȫ�ִ洢һ�����ö���
	logPrefix = prefix;
	logPath = path;
	log_stdOutPut = std_output;
	if (*(logPath.end() - 1) != '/') {
		logPath += "/";
	}
	std::string tempPath = logPath;
	int find = tempPath.find("/");
	uv_fs_t req;
	int result;

	while (find != std::string::npos)
	{
		result = uv_fs_mkdir(uv_default_loop(), &req, tempPath.substr(0, find).c_str(), 0755, NULL);
		find = tempPath.find("/", find + 1);
	}
	uv_fs_req_cleanup(&req);
}

void Logger::log(const char* fileName, int fileLine, int logLevel, const char* msgLog, ...)
{
	//׼���ļ�
	prepareFile();
	//׼����ǰʱ��
	format_time();

	static char msgMetaInfo[1024] = { 0 };
	static char msgContent[10240] = { 0 };
	static char msgTurnLine = '\n';
	//��ȡ��������
	va_list args;
	va_start(args, msgLog);
	vsnprintf(msgContent, sizeof(msgContent), msgLog, args);
	va_end(args);

	sprintf(msgMetaInfo, "%s:%u", fileName, fileLine);

	uv_buf_t buf[7];
	//��־���
	//��һ����----ʱ��
	buf[0] = uv_buf_init(gFormatTime, strlen(gFormatTime));
	//�ڶ�����----��־��־�ȼ�
	buf[1] = uv_buf_init(gLogLevel[logLevel], strlen(gLogLevel[logLevel]));
	//��������----������Ϣ���ļ���������
	buf[2] = uv_buf_init(msgMetaInfo, strlen(msgMetaInfo));
	//���Ĳ���----����
	buf[3] = uv_buf_init(&msgTurnLine, 1);
	//���岿��----����
	buf[4] = uv_buf_init(msgContent, strlen(msgContent));
	//���岿��----����
	buf[5] = uv_buf_init(&msgTurnLine, 1);
	//���岿��----����
	buf[6] = uv_buf_init(&msgTurnLine, 1);

	uv_fs_t printLog;

	int result = uv_fs_write(NULL, &printLog, gFileHandle.result, buf, sizeof(buf) / sizeof(buf[0]), -1, NULL);
	if (result < 0) {
		fprintf(stderr, "log failed %s%s%s%s", gLastSecond, gLogLevel[logLevel], msgMetaInfo, msgContent);
	}

	uv_fs_req_cleanup(&printLog);

	if (log_stdOutPut) {
		printf("%s:%u\n[%s] %s\n\n", fileName, fileLine, gLogLevel[logLevel], msgContent);
	}
}