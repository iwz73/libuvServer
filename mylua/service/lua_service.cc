#include "luainterface/luainterface.h"
#include "utils/logger.h"
#include "netbus/servicemanager.h"
#include "lua_service.h"
using namespace google::protobuf;

//�Լ�luaȫ��ע�ắ����
#define SERVICEFUNCMAP "SERVICEFUNCMAP"

static unsigned int luaFuncId = 0;

static void initServiceFuncMap(lua_State* L)
{
	//pushһ��map����
	lua_pushstring(L, SERVICEFUNCMAP);
	//���������
	lua_newtable(L);
	//ע�������
	lua_rawset(L, LUA_REGISTRYINDEX);
}

static unsigned int saveServiceFunc(lua_State* L, int lo, int def)
{
	//�������-1 ���ⲻ��lua����
	if (!lua_isfunction(L, lo))return -1;

	luaFuncId++;

	lua_pushstring(L, SERVICEFUNCMAP);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushinteger(L, luaFuncId);
	lua_pushvalue(L, lo);
	//���ղ�push��3��ջ ���ϳ�һ��ӳ��
	lua_rawset(L, -3);

	lua_pop(L, 1);

	return luaFuncId;
}

static void getFunctionByFuncId(lua_State* L, int funcId)
{
	lua_pushstring(L, SERVICEFUNCMAP);
	lua_rawget(L, LUA_REGISTRYINDEX);                           /* stack: ... refid_fun */
	lua_pushinteger(L, funcId);                                  /* stack: ... refid_fun refid */
	lua_rawget(L, -2);                                          /* stack: ... refid_fun fun */
	lua_remove(L, -2);                                          /* stack: ... fun */
}

//��ȫ�ֱ��������nhandle���id��Ӧ�ĺ���
static bool findLuaFuncByHandle(int nHandle)
{
	getFunctionByFuncId(LuaInterface::getGlobalState(), nHandle);
	if (!lua_isfunction(LuaInterface::getGlobalState(), -1))
	{
		logError("[LUA ERROR] function refid '%d' does not reference a Lua function", nHandle);
		lua_pop(LuaInterface::getGlobalState(), 1);
		return false;
	}
	return true;
}


static int exeLuaFuncByFuncId(int numArgs)
{
	int functionIndex = -(numArgs + 1);
	if (!lua_isfunction(LuaInterface::getGlobalState(), functionIndex))
	{
		logError("value at stack [%d] is not function", functionIndex);
		lua_pop(LuaInterface::getGlobalState(), numArgs + 1); // remove function and arguments
		return 0;
	}

	int traceback = 0;
	lua_getglobal(LuaInterface::getGlobalState(), "__G__TRACKBACK__");                         /* L: ... func arg1 arg2 ... G */
	if (!lua_isfunction(LuaInterface::getGlobalState(), -1))
	{
		lua_pop(LuaInterface::getGlobalState(), 1);                                            /* L: ... func arg1 arg2 ... */
	}
	else
	{
		lua_insert(LuaInterface::getGlobalState(), functionIndex - 1);                         /* L: ... G func arg1 arg2 ... */
		traceback = functionIndex - 1;
	}

	int error = 0;
	error = lua_pcall(LuaInterface::getGlobalState(), numArgs, 1, traceback);                  /* L: ... [G] ret */
	if (error)
	{
		if (traceback == 0)
		{
			logError("[LUA ERROR] %s", lua_tostring(LuaInterface::getGlobalState(), -1));        /* L: ... error */
			lua_pop(LuaInterface::getGlobalState(), 1); // remove error message from stack
		}
		else                                                            /* L: ... G error */
		{
			lua_pop(LuaInterface::getGlobalState(), 2); // remove __G__TRACKBACK__ and error message from stack
		}
		return 0;
	}

	// get return value
	int ret = 0;
	if (lua_isnumber(LuaInterface::getGlobalState(), -1))
	{
		ret = (int)lua_tointeger(LuaInterface::getGlobalState(), -1);
	}
	else if (lua_isboolean(LuaInterface::getGlobalState(), -1))
	{
		ret = (int)lua_toboolean(LuaInterface::getGlobalState(), -1);
	}
	// remove return value from stack
	lua_pop(LuaInterface::getGlobalState(), 1);                                                /* L: ... [G] */

	if (traceback)
	{
		lua_pop(LuaInterface::getGlobalState(), 1); // remove __G__TRACKBACK__ from stack      /* L: ... */
	}

	return ret;
}

static int executeServiceFunc(int nHandler, int numArgs)
{
	int ret = 0;
	if (findLuaFuncByHandle(nHandler))                                /* L: ... arg1 arg2 ... func */
	{
		if (numArgs > 0)
		{
			lua_insert(LuaInterface::getGlobalState(), -(numArgs + 1));                        /* L: ... func arg1 arg2 ... */
		}
		ret = exeLuaFuncByFuncId(numArgs);
	}
	lua_settop(LuaInterface::getGlobalState(), 0);
	return ret;
}

static int registService(lua_State* L)
{
	int sType = tolua_tonumber(L, 1, 0);
	//�ж��Ƿ�Ϊ��
	if (lua_istable(L, 2))
	{
		int ret;
		//������������
		ret = lua_getfield(L, 2, "on_recv");
		ret = lua_getfield(L, 2, "on_discon");
		unsigned int luaRecvCmdHandle;
		unsigned int luaDisconHandle;
		//��������ջ֮���ջ��λ�ñ����3��4 
		//tip�������ʱ��Ҫ��ע���Լ��ķ����
		luaRecvCmdHandle = saveServiceFunc(L, 3, 0);
		luaDisconHandle = saveServiceFunc(L, 4, 0);

		if (luaRecvCmdHandle || luaDisconHandle)
		{
			lua_service* luaService = new lua_service();
			luaService->luaRecvCmdHandle = luaRecvCmdHandle;
			luaService->luaDisconHandle = luaDisconHandle;
			luaService->luaRecvRawHandle = 0;
			luaService->using_raw = false;
			//ע�Ὺʼ������״̬
			lua_pushboolean(L, ServiceManager::registService(sType, (Service*)luaService) ? 1 : 0);
		}
		return 1;
	}
	lua_pushboolean(L, 0);
	return 1;
}

static int repeaterService(lua_State* L)
{
	int sType = tolua_tonumber(L, 1, 0);
	//�ж��Ƿ�Ϊ��
	if (lua_istable(L, 2))
	{
		int ret;
		//������������
		ret = lua_getfield(L, 2, "repeatCmd");
		ret = lua_getfield(L, 2, "gw_disc");
		unsigned int luaRecvRawHandle;
		unsigned int luaDisconHandle;
		//��������ջ֮���ջ��λ�ñ����3��4 
		//tip�������ʱ��Ҫ��ע���Լ��ķ����
		luaRecvRawHandle = saveServiceFunc(L, 3, 0);
		luaDisconHandle = saveServiceFunc(L, 4, 0);

		if (luaRecvRawHandle || luaDisconHandle)
		{
			lua_service* luaService = new lua_service();
			luaService->luaRecvRawHandle = luaRecvRawHandle;
			luaService->luaDisconHandle = luaDisconHandle;
			luaService->luaRecvCmdHandle = 0;
			luaService->using_raw = true;
			//ע�Ὺʼ������״̬
			lua_pushboolean(L, ServiceManager::registService(sType, (Service*)luaService) ? 1 : 0);
		}
		return 1;
	}
	lua_pushboolean(L, 0);
	return 1;
}

static void
push_proto_message_tolua(const Message* message) {
	lua_State* state = LuaInterface::getGlobalState();
	if (!message) {
		// printf("PushProtobuf2LuaTable failed, message is NULL");
		return;
	}
	const Reflection* reflection = message->GetReflection();

	// ����table
	lua_newtable(state);

	const Descriptor* descriptor = message->GetDescriptor();
	for (int32_t index = 0; index < descriptor->field_count(); ++index) {
		const FieldDescriptor* fd = descriptor->field(index);
		const std::string& name = fd->name();

		// key
		lua_pushstring(state, name.c_str());

		bool bReapeted = fd->is_repeated();

		if (bReapeted) {
			// repeated����table
			lua_newtable(state);
			int size = reflection->FieldSize(*message, fd);
			for (int i = 0; i < size; ++i) {
				char str[32] = { 0 };
				switch (fd->cpp_type()) {
				case FieldDescriptor::CPPTYPE_DOUBLE:
					lua_pushnumber(state, reflection->GetRepeatedDouble(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					lua_pushnumber(state, (double)reflection->GetRepeatedFloat(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_INT64:
					sprintf(str, "%lld", (long long)reflection->GetRepeatedInt64(*message, fd, i));
					lua_pushstring(state, str);
					break;
				case FieldDescriptor::CPPTYPE_UINT64:

					sprintf(str, "%llu", (unsigned long long)reflection->GetRepeatedUInt64(*message, fd, i));
					lua_pushstring(state, str);
					break;
				case FieldDescriptor::CPPTYPE_ENUM: // ��int32һ������
					lua_pushinteger(state, reflection->GetRepeatedEnum(*message, fd, i)->number());
					break;
				case FieldDescriptor::CPPTYPE_INT32:
					lua_pushinteger(state, reflection->GetRepeatedInt32(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_UINT32:
					lua_pushinteger(state, reflection->GetRepeatedUInt32(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_STRING:
				{
					std::string value = reflection->GetRepeatedString(*message, fd, i);
					lua_pushlstring(state, value.c_str(), value.size());
				}
				break;
				case FieldDescriptor::CPPTYPE_BOOL:
					lua_pushboolean(state, reflection->GetRepeatedBool(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
					push_proto_message_tolua(&(reflection->GetRepeatedMessage(*message, fd, i)));
					break;
				default:
					break;
				}

				lua_rawseti(state, -2, i + 1); // lua's index start at 1
			}

		}
		else {
			char str[32] = { 0 };
			switch (fd->cpp_type()) {

			case FieldDescriptor::CPPTYPE_DOUBLE:
				lua_pushnumber(state, reflection->GetDouble(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_FLOAT:
				lua_pushnumber(state, (double)reflection->GetFloat(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_INT64:

				sprintf(str, "%lld", (long long)reflection->GetInt64(*message, fd));
				lua_pushstring(state, str);
				break;
			case FieldDescriptor::CPPTYPE_UINT64:

				sprintf(str, "%llu", (unsigned long long)reflection->GetUInt64(*message, fd));
				lua_pushstring(state, str);
				break;
			case FieldDescriptor::CPPTYPE_ENUM: // ��int32һ������
				lua_pushinteger(state, (int)reflection->GetEnum(*message, fd)->number());
				break;
			case FieldDescriptor::CPPTYPE_INT32:
				lua_pushinteger(state, reflection->GetInt32(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_UINT32:
				lua_pushinteger(state, reflection->GetUInt32(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_STRING:
			{
				std::string value = reflection->GetString(*message, fd);
				lua_pushlstring(state, value.c_str(), value.size());
			}
			break;
			case FieldDescriptor::CPPTYPE_BOOL:
				lua_pushboolean(state, reflection->GetBool(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_MESSAGE:
				push_proto_message_tolua(&(reflection->GetMessage(*message, fd)));
				break;
			default:
				break;
			}
		}

		lua_rawset(state, -3);
	}
}

bool lua_service::recvServiceCmd(Session* s, commandMessage* msg)
{
	tolua_pushuserdata(LuaInterface::getGlobalState(), (void*)s);
	int index = 1;

	lua_newtable(LuaInterface::getGlobalState());
	lua_pushinteger(LuaInterface::getGlobalState(), msg->serverType);
	lua_rawseti(LuaInterface::getGlobalState(), -2, index);          /* table[index] = value, L: table */
	++index;

	lua_pushinteger(LuaInterface::getGlobalState(), msg->commandType);
	lua_rawseti(LuaInterface::getGlobalState(), -2, index);          /* table[index] = value, L: table */
	++index;

	lua_pushinteger(LuaInterface::getGlobalState(), msg->tags);
	lua_rawseti(LuaInterface::getGlobalState(), -2, index);          /* table[index] = value, L: table */
	++index;

	if (!msg->body) {
		lua_pushnil(LuaInterface::getGlobalState());
		lua_rawseti(LuaInterface::getGlobalState(), -2, index);          /* table[index] = value, L: table */
		++index;
	}
	else {
		if (protocolManager::getProtocolType() == PROTOCOL_JSON) {
			lua_pushstring(LuaInterface::getGlobalState(), (char*)msg->body);
		}
		else { // protobuf
			push_proto_message_tolua((Message*)msg->body);
		}
		lua_rawseti(LuaInterface::getGlobalState(), -2, index);          /* table[index] = value, L: table */
		++index;
	}

	executeServiceFunc(this->luaRecvCmdHandle, 2);
	return true;
}

bool lua_service::recvServiceRawHead(Session* s, Gateway2Server* msg)
{
	tolua_pushuserdata(LuaInterface::getGlobalState(), (void*)s);
	tolua_pushuserdata(LuaInterface::getGlobalState(), (void*)msg);
	executeServiceFunc(this->luaRecvRawHandle, 2);
	return true;
}

bool lua_service::disconnectService(Session* s, int sType)
{
	tolua_pushuserdata(LuaInterface::getGlobalState(), (void*)s);
	tolua_pushnumber(LuaInterface::getGlobalState(), sType);
	executeServiceFunc(this->luaDisconHandle,2);
	return true;
}

static int readHead(lua_State* L)
{
	Gateway2Server* rawData = (Gateway2Server*)tolua_touserdata(L, 1, 0);
	if (rawData)
	{
		lua_pushinteger(L, rawData->serverType);
		lua_pushinteger(L, rawData->commandType);
		lua_pushinteger(L, rawData->tags);
		return 3;
	}
	return 0;
}

static int readBody(lua_State* L)
{
	Gateway2Server* rawData = (Gateway2Server*)tolua_touserdata(L, 1, 0);
	if (rawData)
	{
		commandMessage* msg = NULL;
		if (protocolManager::decodeCmdMsg(rawData->rawCmd, rawData->rawLen, &msg))
		{
			if (msg->body == NULL)
			{
				lua_pushnil(L);
			}
			else if (protocolManager::getProtocolType() == PROTOCOL_JSON)
			{
				lua_pushstring(L, (const char*)msg->body);
			}
			else
			{
				push_proto_message_tolua((Message*)msg->body);
			}
			protocolManager::freeDecodeCmdMsg(msg);
			return 1;
		}
	}
	return 0;
}

static int setTag(lua_State* L)
{
	Gateway2Server* rawData = (Gateway2Server*)tolua_touserdata(L, 1, 0);
	if (rawData)
	{
		unsigned int utag = (unsigned int)tolua_tonumber(L, 2, 0);
		rawData->tags = utag;
		unsigned char* pTag = rawData->rawCmd + 4;
		pTag[0] = (utag & 0x000000FF);
		pTag[1] = ((utag & 0x0000FF00)>>8);
		pTag[2] = ((utag & 0x00FF0000)>>16);
		pTag[3] = ((utag & 0xFF000000)>>24);
	}
	return 0;
}

int regist_service_export(lua_State* L)
{
	initServiceFuncMap(L);
	//_G��lua�����е�ȫ�ֱ�
	lua_getglobal(L, "_G");
	if (lua_istable(L, -1))
	{
		tolua_open(L);
		//��ʾ��
		tolua_module(L, "ServiceManager", 0);
		tolua_beginmodule(L, "ServiceManager");
		//��ʾ����
		tolua_function(L, "registService", registService);
		tolua_function(L, "repeaterService", repeaterService);
		tolua_endmodule(L);
	}

	lua_pop(L, 1);
	return 0;
}

int regist_releaseRaw_export(lua_State* L)
{
	initServiceFuncMap(L);
	//_G��lua�����е�ȫ�ֱ�
	lua_getglobal(L, "_G");
	if (lua_istable(L, -1))
	{
		tolua_open(L);
		//��ʾ��
		tolua_module(L, "Raw", 0);
		tolua_beginmodule(L, "Raw");
		//��ʾ����
		tolua_function(L, "readHead", readHead);
		tolua_function(L, "readBody", readBody);
		tolua_function(L, "setTag", setTag);
		tolua_endmodule(L);
	}

	lua_pop(L, 1);
	return 0;
}