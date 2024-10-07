#include "hookClass.h"

#include "dyncall/dyncall_callback.h"
#include "spdlog/spdlog.h"
#include "../JSManager.h"
#include "../hook/HookManager.h"
#include "../nativePoint/nativePointClass.h"

#include <unordered_map>

static std::unordered_map<uintptr_t, std::pair<JSValue, JSValue>> hooktable{};

static std::vector<hookClass*> thisdispose{};

struct NativeUserData
{
	HookInstance* hookinfo = nullptr;
	/**
	 * @brief 调用约定
	 */
	std::vector<NativeTypes> agreeOn{};
	JSValue hookFun{};
	DCCallVM* vm = nullptr;
	NativeUserData() {
		vm = dcNewCallVM(1024);
		dcMode(vm, DC_CALL_C_DEFAULT);
		dcReset(vm);
	}
	~NativeUserData() {
		dcFree(vm);
		JS_FreeValue(JSManager::getInstance()->getctx(), hookFun);
	}
};



namespace {
	static JSClassID id;
	static JSClassDef _hookClass = {
		.class_name{"HookBase"},
		.finalizer{[](JSRuntime* rt, JSValue val) {
				auto hook = (hookClass*)JS_GetOpaque(val, id);
				//delete hook;
				// 经检测，释放会出问题, 所以统一程序结束后 或者手动释放dll时释放
			}
		}
	};
}

static char JSNativecall(DCCallback* cb, DCArgs* args, DCValue* result, void* userdata);


hookClass::hookClass() {}

hookClass::~hookClass() {
	delete m_userData;
}


void hookClass::Reg() {
	JSContext* ctx = JSManager::getInstance()->getctx();
	auto rt = JS_GetRuntime(ctx);
	JS_NewClassID(&id);
	JS_NewClass(rt, id, &_hookClass);

	JSValue protoInstance = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, protoInstance, "hook", JS_NewCFunction(ctx, hookClass::hook, "hook", 0));
	JS_SetPropertyStr(ctx, protoInstance, "unhook", JS_NewCFunction(ctx, hookClass::unhook, "unhook", 0));

	JSValue ctroInstance = JS_NewCFunction2(ctx, &hookClass::constructor, _hookClass.class_name, 0, JS_CFUNC_constructor, 0);
	JS_SetConstructor(ctx, ctroInstance, protoInstance);
	JS_SetClassProto(ctx, id, protoInstance);


	JSValue global_obj = JS_GetGlobalObject(ctx);

	JSValue nativeTypes = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, global_obj, "NativeTypes", nativeTypes);
	JS_SetPropertyStr(ctx, nativeTypes, "Void", JS_NewInt32(ctx, (int)NativeTypes::Void));
	JS_SetPropertyStr(ctx, nativeTypes, "Bool", JS_NewInt32(ctx, (int)NativeTypes::Bool));
	JS_SetPropertyStr(ctx, nativeTypes, "Char", JS_NewInt32(ctx, (int)NativeTypes::Char));
	JS_SetPropertyStr(ctx, nativeTypes, "UnsignedChar", JS_NewInt32(ctx, (int)NativeTypes::UnsignedChar));
	JS_SetPropertyStr(ctx, nativeTypes, "Short", JS_NewInt32(ctx, (int)NativeTypes::Short));
	JS_SetPropertyStr(ctx, nativeTypes, "UnsignedShort", JS_NewInt32(ctx, (int)NativeTypes::UnsignedShort));
	JS_SetPropertyStr(ctx, nativeTypes, "Int", JS_NewInt32(ctx, (int)NativeTypes::Int));
	JS_SetPropertyStr(ctx, nativeTypes, "UnsignedInt", JS_NewInt32(ctx, (int)NativeTypes::UnsignedInt));
	JS_SetPropertyStr(ctx, nativeTypes, "Long", JS_NewInt32(ctx, (int)NativeTypes::Long));
	JS_SetPropertyStr(ctx, nativeTypes, "UnsignedLong", JS_NewInt32(ctx, (int)NativeTypes::UnsignedLong));
	JS_SetPropertyStr(ctx, nativeTypes, "LongLong", JS_NewInt32(ctx, (int)NativeTypes::LongLong));
	JS_SetPropertyStr(ctx, nativeTypes, "UnsignedLongLong", JS_NewInt32(ctx, (int)NativeTypes::UnsignedLongLong));
	JS_SetPropertyStr(ctx, nativeTypes, "Float", JS_NewInt32(ctx, (int)NativeTypes::Float));
	JS_SetPropertyStr(ctx, nativeTypes, "Double", JS_NewInt32(ctx, (int)NativeTypes::Double));
	JS_SetPropertyStr(ctx, nativeTypes, "Pointer", JS_NewInt32(ctx, (int)NativeTypes::Pointer));
	

	JS_SetPropertyStr(ctx, global_obj, _hookClass.class_name, ctroInstance);
	JS_FreeValue(ctx, global_obj);
}

void hookClass::Dispose() {
	for(auto it : thisdispose) {
		delete it;
		it = nullptr;
	}
	thisdispose.clear();
}

JSValue hookClass::constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {

	// 第一个参数是回调函数
	//	回调函数参数是参数列表
	// 第二个参数是指针
	// 第三个参数是调用约定 [void(第一个为返回值类型),void*,bool(1字节),short(2字节),int(4字节),float(4字节),long(8字节),double(8字节)]

	if(argc < 3) {
		return JS_ThrowTypeError(ctx, "函数需要3个参数，当前参数个数：%d", argc);
	}
	if(!JS_IsFunction(ctx, argv[0])) {
		return JS_ThrowTypeError(ctx, "参数一应是回调函数");
	}
	int64_t ptr;
	if(!JS_IsNumber(argv[1]) || JS_ToInt64(ctx, &ptr, argv[1]) < 0) {
		return JS_ThrowTypeError(ctx, "参数二应为指针，类型应为Number");
	}
	if(ptr <= 0) {
		return JS_ThrowTypeError(ctx, "参数二指针不能小于等于0");
	}
	if(!JS_IsArray(ctx, argv[2])) {
		return JS_ThrowTypeError(ctx, "参数三作为一个调用约定应是一个NativeTypes数组");
	}

	auto self = new hookClass;
	thisdispose.push_back(self);

	self->m_userData = new NativeUserData();
	self->m_userData->agreeOn.clear();
	self->m_userData->hookFun = JS_DupValue(ctx, argv[0]);

	JSValue lengthVal = JS_GetPropertyStr(ctx, argv[2], "length");
	uint64_t len = 0;
	JS_ToIndex(ctx, &len, lengthVal);
	JS_FreeValue(ctx, lengthVal);
	for(int i = 0; i < len; i++) {
		JSValue item = JS_GetPropertyUint32(ctx, argv[2], i);
		int32_t value = 0;
		int r = JS_ToInt32(ctx, &value, item);
		JS_FreeValue(ctx, item);
		if(r < 0) {
			return JS_ThrowTypeError(ctx, "参数三NativeTypes数组解析失败");
		}
		self->m_userData->agreeOn.push_back((NativeTypes)value);
	}

	auto sign = self->signature();
	self->m_hookinfo = HookManager::addHook(ptr, (void*)dcbNewCallback(sign.c_str(), (DCCallbackHandler*)&JSNativecall, self->m_userData));
	self->m_userData->hookinfo = self->m_hookinfo;
	
	JSValue obj = JS_NewObjectClass(ctx, id);
	JS_SetOpaque(obj, self);
	return obj;
}

JSValue hookClass::hook(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	hookClass* thi = (hookClass*)JS_GetOpaque(newTarget, id);
	thi->m_hookinfo->hook();
	JS_SetPropertyStr(ctx, newTarget, "origin", nativePointClass::newNativePoint((uintptr_t)thi->m_hookinfo->origin, thi->m_userData->agreeOn));
	return JS_UNDEFINED;
}

JSValue hookClass::unhook(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	hookClass* thi = (hookClass*)JS_GetOpaque(newTarget, id);
	thi->m_hookinfo->unhook();
	return JS_UNDEFINED;
}

static char JSNativecall(DCCallback* cb, DCArgs* args, DCValue* result, void* userdata) {
	auto userData = (NativeUserData*)userdata;
	JSContext* ctx = JSManager::getInstance()->getctx();

	try {
		std::vector<JSValue> paras;
		for(int i = 1; i < userData->agreeOn.size(); ++i) {
			switch(userData->agreeOn[i]) {
			case NativeTypes::Bool:
				paras.emplace_back(JS_NewBool(ctx, dcbArgBool(args)));
				break;
			case NativeTypes::Char:
				paras.emplace_back(JS_NewInt32(ctx, dcbArgChar(args)));
				break;
			case NativeTypes::UnsignedChar:
				paras.emplace_back(JS_NewInt32(ctx, dcbArgUChar(args)));
				break;
			case NativeTypes::Short:
				paras.emplace_back(JS_NewInt64(ctx, dcbArgShort(args)));
				break;
			case NativeTypes::UnsignedShort:
				paras.emplace_back(JS_NewInt64(ctx, dcbArgUShort(args)));
				break;
			case NativeTypes::Int:
				paras.emplace_back(JS_NewInt64(ctx, dcbArgInt(args)));
				break;
			case NativeTypes::UnsignedInt:
				paras.emplace_back(JS_NewInt64(ctx, (int)dcbArgUInt(args)));
				break;
			case NativeTypes::Long:
				paras.emplace_back(JS_NewInt64(ctx, (int)dcbArgLong(args)));
				break;
			case NativeTypes::UnsignedLong:
				paras.emplace_back(JS_NewInt64(ctx, (int)dcbArgULong(args)));
				break;
			case NativeTypes::LongLong:
				paras.emplace_back(JS_NewInt64(ctx, dcbArgLongLong(args)));
				break;
			case NativeTypes::UnsignedLongLong:
				paras.emplace_back(JS_NewInt64(ctx, (long long)dcbArgULongLong(args)));
				break;
			case NativeTypes::Float:
				paras.emplace_back(JS_NewFloat64(ctx, dcbArgFloat(args)));
				break;
			case NativeTypes::Double:
				paras.emplace_back(JS_NewFloat64(ctx, dcbArgDouble(args)));
				break;
			case NativeTypes::Pointer:
			{
				uintptr_t point = (uintptr_t)dcbArgPointer(args);
				//paras.emplace_back(nativePointClass::newNativePoint((uintptr_t)dcbArgPointer(args)));
				paras.emplace_back(nativePointClass::newNativePoint(point));
			}
				break;
			default:
				break;
			}
		}
		
		// 然后调用JS中的CALL
		auto ret = JS_Call(ctx, userData->hookFun, JS_GetGlobalObject(ctx), static_cast<int>(paras.size()), paras.data());
		for(int i = 0; i < paras.size(); ++i) {
			JS_FreeValue(ctx, paras[i]);
		}

		// 然后返回值类型去解析返回值，将返回值写入到 result 返回
		switch(userData->agreeOn[0]) {
		case NativeTypes::Bool:
			result->c = JS_ToBool(ctx, ret);
			break;
		case NativeTypes::Char:
		{
			int32_t value = 0;
			if(JS_ToInt32(ctx, &value, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::Char)解析失败");
			}
			result->c = (char)value;
		}
		break;
		case NativeTypes::UnsignedChar:
		{
			int32_t value = 0;
			if(JS_ToInt32(ctx, &value, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::UnsignedChar)解析失败");
			}
			result->C = (unsigned char)value;
		}
		break;
		case NativeTypes::Short:
		{
			int32_t value = 0;
			if(JS_ToInt32(ctx, &value, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::Short)解析失败");
			}
			result->s = (short)value;
		}
		break;
		case NativeTypes::UnsignedShort:
		{
			int32_t value = 0;
			if(JS_ToInt32(ctx, &value, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::UnsignedShort)解析失败");
			}
			result->S = (unsigned)value;
		}
		break;
		case NativeTypes::Int:
		{
			int32_t value = 0;
			if(JS_ToInt32(ctx, &value, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::Int)解析失败");
			}
			result->i = (int)value;
		}
		break;
		case NativeTypes::UnsignedInt:
		{
			int32_t value = 0;
			if(JS_ToInt32(ctx, &value, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::UnsignedInt)解析失败");
			}
			result->I = (int)value;
		}
		break;
		case NativeTypes::Long:
		{
			int32_t value = 0;
			if(JS_ToInt32(ctx, &value, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::Long)解析失败");
			}
			result->j = (long)value;
		}
		break;
		case NativeTypes::UnsignedLong:
		{
			int32_t value = 0;
			if(JS_ToInt32(ctx, &value, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::UnsignedLong)解析失败");
			}
			result->J = (unsigned long)value;
		}
		break;
		case NativeTypes::LongLong:
		{
			int64_t value64 = 0;
			if(JS_ToInt64(ctx, &value64, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::LongLong)解析失败");
			}
			result->l = (long long)value64;
		}
		break;
		case NativeTypes::UnsignedLongLong:
		{
			int64_t value64 = 0;
			if(JS_ToInt64(ctx, &value64, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::UnsignedLongLong)解析失败");
			}
			result->L = (unsigned long long)value64;
		}
		break;
		case NativeTypes::Float:
		{
			double valuef = 0;
			if(JS_ToFloat64(ctx, &valuef, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::Float)解析失败");
			}
			result->f = (float)valuef;
		}
		break;
		case NativeTypes::Double:
		{
			double valued = 0;
			if(JS_ToFloat64(ctx, &valued, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::Double)解析失败");
			}
			result->d = (double)valued;
		}
		break;
		case NativeTypes::Pointer:
		{
			int64_t valuep = 0;
			if(JS_ToInt64(ctx, &valuep, ret) < 0) {
				throw std::runtime_error("返回值(NativeTypes::Pointer)解析失败");
			}
			result->p = (JS_IsNull(ret) || JS_IsUndefined(ret) || valuep == 0)? 0 : (void*)((nativePointClass*)valuep)->get();
		}
		break;
		default:
			break;
		}
		JS_FreeValue(ctx, ret);
	}
	catch(std::runtime_error& re) {
		spdlog::error(re.what());
		spdlog::error("错误发生在-函数：{}，文件：{}", __FUNCTION__, __FILE__);
		throw re;
	}
	catch(std::exception& e) {
		spdlog::error(e.what());
		spdlog::error("错误发生在-函数：{}，文件：{}:", __FUNCTION__, __FILE__,__LINE__);
	}

	return hookClass::getTypeSignature(userData->agreeOn[0]);
}

const std::string hookClass::signature() {
	//return "nullptr";
	return std::to_string((uintptr_t)this);
}


char hookClass::getTypeSignature(NativeTypes type) {
	switch(type) {
	case NativeTypes::Bool:
		return 'B';
		break;
	case NativeTypes::Char:
		return 'c';
		break;
	case NativeTypes::UnsignedChar:
		return 'C';
		break;
	case NativeTypes::Short:
		return 's';
		break;
	case NativeTypes::UnsignedShort:
		return 'S';
		break;
	case NativeTypes::Int:
		return 'i';
		break;
	case NativeTypes::UnsignedInt:
		return 'I';
		break;
	case NativeTypes::Long:
		return 'j';
		break;
	case NativeTypes::UnsignedLong:
		return 'J';
		break;
	case NativeTypes::LongLong:
		return 'l';
		break;
	case NativeTypes::UnsignedLongLong:
		return 'L';
		break;
	case NativeTypes::Float:
		return 'f';
		break;
	case NativeTypes::Double:
		return 'd';
		break;
	case NativeTypes::Pointer:
		return 'p';
		break;
	default:
		return 'v';
		break;
	}
}