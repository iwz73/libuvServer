#include "luainterface/luainterface.h"
#include "lua_schedule.h"
#include "utils/time.h"
#include "utils/Cache.h"

struct scheduleInfo
{
	//����Ļص�����handle
	int handle;
	//����
	int repeatCount;
};

void luaScheduler(void* uData)
{
	scheduleInfo* tr = (scheduleInfo*)uData;
	LuaInterface::executeLuaFunc(tr->handle, 0);
	if (tr->repeatCount == -1) {
		return;
	}

	tr->repeatCount--;
	if (tr->repeatCount <= 0) {
		LuaInterface::removeLuaFunc(tr->handle);
		smallCacheFree(tr);
	}
}

static int lua_scheduleRepeat(lua_State* L)
{
	//�ص�������ʶ
	int handle = toluafix_ref_function(L, 1, 0);
	//����ms��ʼ����
	int afterMsec = lua_tointeger(L, 2);
	//�������ٴ�
	int repeatCount = lua_tointeger(L, 3);
	//ÿ������������
	int repeatMsec = lua_tointeger(L, 4);

	if (handle && afterMsec && repeatCount)
	{
		if (!repeatMsec)
			repeatMsec = afterMsec;
		scheduleInfo* ts = (scheduleInfo*)smallCacheAllocer(sizeof(scheduleInfo));
		ts->handle = handle;
		ts->repeatCount = repeatCount;
		tolua_pushuserdata(L, scheduleRepeat(luaScheduler, ts, afterMsec, ts->repeatCount, repeatMsec));
		return 1;
	}
	if (handle != 0)
		LuaInterface::removeLuaFunc(handle);
	lua_pushnil(L);

	return 1;
}

static int lua_cancelTimer(lua_State* L)
{
	if (lua_isuserdata(L, 1))
	{
		timer* scheduler = (timer*)lua_touserdata(L, 1);
		scheduleInfo* tr = (scheduleInfo*)get_timer_udata(scheduler);
		LuaInterface::removeLuaFunc(tr->handle);
		smallCacheFree(tr);
		cancelTimer(scheduler);
	}
	return 0;
}

static int lua_scheduleOnce(lua_State* L)
{
	int handle = toluafix_ref_function(L, 1, 0);
	int afterMsec = lua_tointeger(L, 2);

	if (handle && afterMsec)
	{
		if (afterMsec > 0)
		{
			scheduleInfo* tr = (scheduleInfo*)smallCacheAllocer(sizeof(scheduleInfo));
			tr->handle = handle;
			tr->repeatCount = 1;
			struct timer* t = scheduleOnce(luaScheduler, (void*)tr, afterMsec);
			tolua_pushuserdata(L, t);
			return 1;
		}
		if (handle != 0)
			LuaInterface::removeLuaFunc(handle);
		lua_pushnil(L);

		return 1;
	}
	if (handle != 0)
		LuaInterface::removeLuaFunc(handle);
	lua_pushnil(L);

	return 1;
}

int regist_schedule_export(lua_State* L)
{
	//_G��lua�����е�ȫ�ֱ�
	lua_getglobal(L, "_G");
	if (lua_istable(L, -1))
	{
		tolua_open(L);
		//��ʾ��
		tolua_module(L, "scheduler", 0);
		tolua_beginmodule(L, "scheduler");
		//��ʾ����
		tolua_function(L, "scheduleRepeat", lua_scheduleRepeat);
		tolua_function(L, "cancelTimer", lua_cancelTimer);
		tolua_function(L, "scheduleOnce", lua_scheduleOnce);
		tolua_endmodule(L);
	}
	lua_pop(L, 1);
	return 0;
}