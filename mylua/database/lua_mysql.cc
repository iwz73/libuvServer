#include "luainterface/luainterface.h"
#include "database/mysqlmanager.h"
#include "lua_mysql.h"

static void mysqlOperationCon(const char* err, void* context, void* uData)
{
	//�д���
	if (err)
	{
		lua_pushstring(LuaInterface::getGlobalState(), err);
		lua_pushnil(LuaInterface::getGlobalState());
	}
	//û����
	else
	{
		lua_pushnil(LuaInterface::getGlobalState());
		tolua_pushuserdata(LuaInterface::getGlobalState(), context);
	}
	LuaInterface::executeLuaFunc((int)uData, 2);
	LuaInterface::removeLuaFunc((int)uData);
}

static int connect(lua_State* L)
{
	char* ip = (char*)tolua_tostring(L, 1, 0);
	int port = tolua_tonumber(L, 2, 0);
	char* db = (char*)tolua_tostring(L, 3, 0);
	char* user = (char*)tolua_tostring(L, 4, 0);
	char* pswd = (char*)tolua_tostring(L, 5, 0);
	//��������
	int handle = toluafix_ref_function(L, 6, 0);
	if (ip && port && db && user && pswd)
		mysqlManager::connect(ip, port, db, user, pswd, mysqlOperationCon,(void*)handle);
	return 0;
}

static int close(lua_State* L)
{
	void* context = tolua_touserdata(LuaInterface::getGlobalState(), 1, 0);
	if (context) {
		mysqlManager::close(context);
	}
	return 0;
}

static void pushMysqlQueryRes(int num, sql::ResultSet* result, sql::ResultSetMetaData* res)
{
	//�ڶ����ӱ��ǻ�ȡ��������
	lua_newtable(LuaInterface::getGlobalState());
	int index = 1;
	sql::SQLString colName;
	sql::SQLString toLua;

	for (int i = 1; i <= num; i++)
	{
		colName = res->getColumnName(i);
		toLua = result->getString(colName.c_str());
		lua_pushstring(LuaInterface::getGlobalState(), toLua.c_str());
		lua_rawseti(LuaInterface::getGlobalState(), -2, index);          /* table[index] = value, L: table */
		++index;
	}
}

static void mysqlOperationQue(const char* err, sql::ResultSet* result,void* uData)
{
	void* context = tolua_touserdata(LuaInterface::getGlobalState(), 1, 0);

	if (err)
	{
		lua_pushstring(LuaInterface::getGlobalState(), err);
		lua_pushnil(LuaInterface::getGlobalState());
	}
	//û����
	else
	{
		lua_pushnil(LuaInterface::getGlobalState());
		if (result)
		{
			//�����push�ĵ�һ������err
			lua_pushnil(LuaInterface::getGlobalState());

			//���ǵڶ�������--queryResul
			int index = 1;
			sql::ResultSetMetaData* res = result->getMetaData();

			//����з��ؽ�� ����һ��lua��������
			lua_newtable(LuaInterface::getGlobalState());

			lua_newtable(LuaInterface::getGlobalState());
			int colInfo = 1;
			for (unsigned int i = 1; i <= res->getColumnCount(); i++)
			{
				lua_pushstring(LuaInterface::getGlobalState(), res->getColumnName(i).c_str());
				lua_rawseti(LuaInterface::getGlobalState(), -2, colInfo);          /* table[index] = value, L: table */
				++colInfo;
			}
			lua_rawseti(LuaInterface::getGlobalState(), -2, index);
			++index;
			while (result->next())
			{
				//��ӡһ�в�ѯ������Ϣ��push
				pushMysqlQueryRes(res->getColumnCount(),result,res);
				lua_rawseti(LuaInterface::getGlobalState(), -2, index);          /* table[index] = value, L: table */
				++index;
			}
		}
		else
		{
			lua_pushstring(LuaInterface::getGlobalState(), "query sqlstatement success");
		}
	}
	LuaInterface::executeLuaFunc((int)uData, 2);
	LuaInterface::removeLuaFunc((int)uData);
}

static int query(lua_State* L)
{
	void* context = tolua_touserdata(L, 1, 0);
	char* sql = (char*)tolua_tostring(L, 2, 0);
	int handler = toluafix_ref_function(L, 3, 0);
	if (context && sql && handler)
		mysqlManager::query(context, sql, mysqlOperationQue, (void*)handler);
	return 0;
}


int regist_mysql_export(lua_State* L)
{
	//_G��lua�����е�ȫ�ֱ�
	lua_getglobal(L, "_G");
	if (lua_istable(L, -1))
	{
		tolua_open(L);
		//��ʾ��
		tolua_module(L, "mysqlManager", 0);
		tolua_beginmodule(L, "mysqlManager");
		//��ʾ����
		tolua_function(L, "connect", connect);
		tolua_function(L, "close", close);
		tolua_function(L, "query", query);

		tolua_endmodule(L);
	}

	lua_pop(L, 1);
	return 0;
}
