#include "nativePointClass.h"
#include "dyncall/dyncall_callback.h"
#include "spdlog/spdlog.h"
#include "../JSManager.h"
#include "../client/mem/mem.h"

#define NOMINMAX

namespace {
	//static JSClassID id;
	static JSClassDef _nativePointClass = {
		.class_name{"NativePoint"},
		.finalizer{[](JSRuntime* rt, JSValue val) {
				auto NativePoint = (nativePointClass*)JS_GetOpaque(val, nativePointClass::id);
				delete NativePoint;
			}
		}
	};
}

JSClassID nativePointClass::id{};

void nativePointClass::Reg() {
	JSContext* ctx = JSManager::getInstance()->getctx();
	auto rt = JS_GetRuntime(ctx);
	JS_NewClassID(&nativePointClass::id);
	JS_NewClass(rt, nativePointClass::id, &_nativePointClass);
	

	JSValue protoInstance = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, protoInstance, "call", JS_NewCFunction(ctx, nativePointClass::call, "call", 0));
	JS_SetPropertyStr(ctx, protoInstance, "offset", JS_NewCFunction(ctx, nativePointClass::offset, "offset", 0));
	JS_SetPropertyStr(ctx, protoInstance, "toNumber", JS_NewCFunction(ctx, nativePointClass::toNumber, "toNumber", 0));
	JS_SetPropertyStr(ctx, protoInstance, "setAgree", JS_NewCFunction(ctx, nativePointClass::setAgree, "setAgree", 0));
	JS_SetPropertyStr(ctx, protoInstance, "setbool", JS_NewCFunction(ctx, nativePointClass::setbool, "setbool", 0));
	JS_SetPropertyStr(ctx, protoInstance, "getbool", JS_NewCFunction(ctx, nativePointClass::getbool, "getbool", 0));
	JS_SetPropertyStr(ctx, protoInstance, "setint", JS_NewCFunction(ctx, nativePointClass::setint, "setint", 0));
	JS_SetPropertyStr(ctx, protoInstance, "getint", JS_NewCFunction(ctx, nativePointClass::getint, "getint", 0));
	JS_SetPropertyStr(ctx, protoInstance, "setlong", JS_NewCFunction(ctx, nativePointClass::setlong, "setlong", 0));
	JS_SetPropertyStr(ctx, protoInstance, "getlong", JS_NewCFunction(ctx, nativePointClass::getlong, "getlong", 0));
	JS_SetPropertyStr(ctx, protoInstance, "setfloat", JS_NewCFunction(ctx, nativePointClass::setfloat, "setfloat", 0));
	JS_SetPropertyStr(ctx, protoInstance, "getfloat", JS_NewCFunction(ctx, nativePointClass::getfloat, "getfloat", 0));
	JS_SetPropertyStr(ctx, protoInstance, "setdouble", JS_NewCFunction(ctx, nativePointClass::setdouble, "setdouble", 0));
	JS_SetPropertyStr(ctx, protoInstance, "getdouble", JS_NewCFunction(ctx, nativePointClass::getdouble, "getdouble", 0));
	JS_SetPropertyStr(ctx, protoInstance, "setpoint", JS_NewCFunction(ctx, nativePointClass::setpoint, "setpoint", 0));
	JS_SetPropertyStr(ctx, protoInstance, "getpoint", JS_NewCFunction(ctx, nativePointClass::getpoint, "getpoint", 0));
	JS_SetPropertyStr(ctx, protoInstance, "getstring", JS_NewCFunction(ctx, nativePointClass::getstring, "getstring", 0));

	JSValue ctroInstance = JS_NewCFunction2(ctx, &nativePointClass::constructor, _nativePointClass.class_name, 0, JS_CFUNC_constructor, 0);
	JS_SetConstructor(ctx, ctroInstance, protoInstance);
	JS_SetClassProto(ctx, id, protoInstance);


	JSValue global_obj = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global_obj, _nativePointClass.class_name, ctroInstance);
	JS_FreeValue(ctx, global_obj);
}

void nativePointClass::Dispose() {}

JSValue nativePointClass::FromPtr(uintptr_t ptr) {
	JSValue obj = JS_NewObjectClass(JSManager::getInstance()->getctx(), id);
	JS_SetOpaque(obj, (void*)ptr);
	return obj;
}

JSValue nativePointClass::newNativePoint(uintptr_t ptr) {
#pragma message( " Warning: 注意这里的野指针问题 " )
	auto thi = new nativePointClass(ptr);
	return FromPtr((uintptr_t)thi);
}

JSValue nativePointClass::newNativePoint(uintptr_t ptr, std::vector<NativeTypes> v) {
#pragma message( " Warning: 注意这里的野指针问题 " )
	auto thi = new nativePointClass(ptr);
	thi->m_agreeOn = v;
	return FromPtr((uintptr_t)thi);
}

nativePointClass::nativePointClass(uintptr_t ptr) {
	m_ptr = ptr;
	m_vm = dcNewCallVM(1024);
	dcMode(m_vm, DC_CALL_C_DEFAULT);
	dcReset(m_vm);
}

nativePointClass::nativePointClass(uintptr_t ptr, UINT calloc) {
	m_ptr = ptr;
	m_vm = dcNewCallVM(1024);
	m_freelen = calloc;
	dcMode(m_vm, DC_CALL_C_DEFAULT);
	dcReset(m_vm);
}

nativePointClass::~nativePointClass() {
	dcFree(m_vm);
	if(m_ptr && m_freelen > 0) {
		free((void*)m_ptr);
		m_freelen = 0;
	}
}

uintptr_t nativePointClass::get() {
	return m_ptr;
}

JSValue nativePointClass::constructor(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	// 至少一个参数 指针
	// 可选第二个参数 当指针作为call时的调用约定
	if(argc < 1) {
		return JS_ThrowTypeError(ctx, "函数至少需要1个参数，当前参数个数：%d", argc);
	}
	// 处理第一个参数
	int64_t ptr;
	if(!JS_IsNumber(argv[0]) || JS_ToInt64(ctx, &ptr, argv[0]) < 0) {
		return JS_ThrowTypeError(ctx, "参数一应为指针，类型应为Number");
	}

	nativePointClass* self = nullptr;
	int64_t memsize = 0;
	if(ptr == 0 && argc >= 2 && JS_IsNumber(argv[1]) && JS_ToInt64(ctx, &memsize, argv[1]) >= 0 && memsize > 0) {
		ptr = (int64_t)calloc(memsize, 1);
		self = new nativePointClass(ptr, static_cast<UINT>(memsize));
	}
	else {
		self = new nativePointClass(ptr);
	}

	if(argc >= 2) {
		// 处理第二个参数
		JSValue lengthVal = JS_GetPropertyStr(ctx, argv[1], "length");
		uint64_t len = 0;
		JS_ToIndex(ctx, &len, lengthVal);
		JS_FreeValue(ctx, lengthVal);
		for(int i = 0; i < len; i++) {
			JSValue item = JS_GetPropertyUint32(ctx, argv[1], i);
			int32_t value = 0;
			int r = JS_ToInt32(ctx, &value, item);
			JS_FreeValue(ctx, item);
			if(r < 0) {
				return JS_ThrowTypeError(ctx, "参数二NativeTypes数组解析失败");
			}
			self->m_agreeOn.push_back((NativeTypes)value);
		}
	}
	return FromPtr((uintptr_t)self);
}

JSValue nativePointClass::offset(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	if(argc < 1) {
		return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
	}
	INT32 offset;
	if(!JS_IsNumber(argv[0]) || JS_ToInt32(ctx, &offset, argv[0]) < 0) {
		return JS_ThrowTypeError(ctx, "参数一应为整数，类型应为Number");
	}
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	// 创建一个副本
	return newNativePoint(thi->m_ptr + offset);
}

JSValue nativePointClass::addvtf(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	return JSValue();
}

JSValue nativePointClass::setAgree(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	// 设置调用约定
	if(argc < 1) {
		return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
	}
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	JSValue lengthVal = JS_GetPropertyStr(ctx, argv[0], "length");
	uint64_t len = 0;
	int r = JS_ToIndex(ctx, &len, lengthVal);
	JS_FreeValue(ctx, lengthVal);
	if(r < 0) {
		return JS_ThrowTypeError(ctx, "参数一NativeTypes数组解析失败");
	}
	thi->m_agreeOn.clear();
	for(int i = 0; i < len; i++) {
		JSValue item = JS_GetPropertyUint32(ctx, argv[0], i);
		int32_t value = 0;
		int r = JS_ToInt32(ctx, &value, item);
		JS_FreeValue(ctx, item);
		if(r < 0) {
			return JS_ThrowTypeError(ctx, "参数二NativeTypes数组解析失败 i: %d", i);
		}
		
		thi->m_agreeOn.push_back((NativeTypes)value);
	}
	return JS_DupValue(ctx, newTarget);
	
}

JSValue nativePointClass::setbool(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	if(argc < 1) {
		return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
	}
	if(!JS_IsBool(argv[0])) {
		return JS_ThrowTypeError(ctx, "参数一应为布尔值，类型应为bool");
	}
	int r = JS_ToBool(ctx, argv[0]);
	if(r < 0) {
		return JS_ThrowTypeError(ctx, "参数一应为布尔值，类型应为bool");
	}
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	//*(bool*)thi->m_ptr = (bool)r;
	Mem::setValue<bool>(thi->m_ptr, r);
	return JS_UNDEFINED;
}

JSValue nativePointClass::getbool(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	return JS_NewBool(ctx, Mem::getValue<bool>(thi->m_ptr));
}

JSValue nativePointClass::setint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	if(argc < 1) {
		return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
	}
	INT32 value;
	if(!JS_IsNumber(argv[0]) || JS_ToInt32(ctx, &value, argv[0]) < 0) {
		return JS_ThrowTypeError(ctx, "参数一应为整数，类型应为Number");
	}
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	Mem::setValue<int>(thi->m_ptr, value);
	return JS_UNDEFINED;
}

JSValue nativePointClass::getint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	return JS_NewInt32(ctx, Mem::getValue<int>(thi->m_ptr));
}

JSValue nativePointClass::setlong(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	if(argc < 1) {
		return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
	}
	INT64 value;
	if(!JS_IsNumber(argv[0]) || JS_ToInt64(ctx, &value, argv[0]) < 0) {
		return JS_ThrowTypeError(ctx, "参数一应为长整数，类型应为Number");
	}
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	Mem::setValue<long long>(thi->m_ptr, value);
	return JS_UNDEFINED;
}

JSValue nativePointClass::getlong(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	return JS_NewInt64(ctx, Mem::getValue<long long>(thi->m_ptr));
}

JSValue nativePointClass::setfloat(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	if(argc < 1) {
		return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
	}
	double value;
	if(JS_ToFloat64(ctx, &value, argv[0]) < 0) {
		return JS_ThrowTypeError(ctx, "参数一应为浮点数，类型应为float");
	}
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	Mem::setValue<float>(thi->m_ptr, static_cast<float>(value));
	return JS_UNDEFINED;
}

JSValue nativePointClass::getfloat(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	return JS_NewFloat64(ctx, (double)Mem::getValue<float>(thi->m_ptr));
}

JSValue nativePointClass::setdouble(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	if(argc < 1) {
		return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
	}
	double value;
	if(JS_ToFloat64(ctx, &value, argv[0]) < 0) {
		return JS_ThrowTypeError(ctx, "参数一应为浮点数，类型应为double");
	}
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	Mem::setValue<double>(thi->m_ptr, value);
	return JS_UNDEFINED;
}

JSValue nativePointClass::getdouble(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	return JS_NewFloat64(ctx, Mem::getValue<double>(thi->m_ptr));
}


JSValue nativePointClass::setpoint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	if(argc < 1) {
		return JS_ThrowTypeError(ctx, "函数需要1个参数，当前参数个数：%d", argc);
	}
	
	nativePointClass* thii = (nativePointClass*)JS_GetOpaque(argv[0], id);
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	Mem::setValue<uintptr_t>(thi->m_ptr, thii->m_ptr);
	return JS_UNDEFINED;
}

JSValue nativePointClass::getpoint(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	auto ptr = Mem::getValue<uintptr_t>(thi->m_ptr);
	return newNativePoint(ptr);
}

JSValue nativePointClass::getstring(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	std::string& str = *(std::string*)thi->m_ptr;
	return JS_NewString(ctx, str.c_str());
}

////////////////////////////// CALL /////////////////////////////////
JSValue nativePointClass::call(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	JSValue ret{};
	try {
		auto ori = (void*)thi->m_ptr;
		dcReset(thi->m_vm);
		for(int i = 1; i < thi->m_agreeOn.size(); ++i) {
			switch(thi->m_agreeOn[i]) {
			case NativeTypes::Bool:
			{
				dcArgBool(thi->m_vm, JS_ToBool(ctx, argv[i - 1]));
			}
			break;
			case NativeTypes::Char:
			case NativeTypes::UnsignedChar:
			{
				int32_t value = 0;
				if(JS_ToInt32(ctx, &value, argv[i - 1]) < 0) {
					throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedChar(Char))解析失败");
				}
				dcArgChar(thi->m_vm, value);
			}
			break;
			case NativeTypes::Short:
			case NativeTypes::UnsignedShort:
			{
				int32_t value = 0;
				if(JS_ToInt32(ctx, &value, argv[i - 1]) < 0) {
					throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedShort(Short))解析失败");
				}
				dcArgShort(thi->m_vm, value);
			}
			break;
			case NativeTypes::Int:
			case NativeTypes::UnsignedInt:
			{
				int32_t value = 0;
				if(JS_ToInt32(ctx, &value, argv[i - 1]) < 0) {
					throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedInt(Int))解析失败");
				}
				dcArgInt(thi->m_vm, value);
			}
			break;
			case NativeTypes::Long:
			case NativeTypes::UnsignedLong:
			{
				int32_t value = 0;
				if(JS_ToInt32(ctx, &value, argv[i - 1]) < 0) {
					throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedLong(Long))解析失败");
				}
				dcArgLong(thi->m_vm, value);
			}
			break;
			case NativeTypes::LongLong:
			case NativeTypes::UnsignedLongLong:
			{
				int64_t value64 = 0;
				if(JS_ToInt64(ctx, &value64, argv[i - 1]) < 0) {
					throw std::runtime_error("NativePoint.call(NativeTypes::UnsignedLongLong(LongLong))解析失败");
				}
				dcArgLongLong(thi->m_vm, value64);
			}
			break;
			case NativeTypes::Float:
			{
				double valuef = 0;
				if(JS_ToFloat64(ctx, &valuef, argv[i - 1]) < 0) {
					throw std::runtime_error("NativePoint.call(NativeTypes::Float)解析失败");
				}
				if(valuef > std::numeric_limits<float>::max()) {
					spdlog::warn("NativePoint.call(NativeTypes::Float)解析时发现值大于float最大值：{}", valuef);
				}
				dcArgFloat(thi->m_vm, static_cast<DCfloat>(valuef));
			}
			break;
			case NativeTypes::Double:
			{
				double valued = 0;
				if(JS_ToFloat64(ctx, &valued, argv[i - 1]) < 0) {
					throw std::runtime_error("NativePoint.call(NativeTypes::Double)解析失败");
				}
				dcArgDouble(thi->m_vm, valued);
			}
			break;
			case NativeTypes::Pointer:
			{
				//int64_t valuepoint = 0;
				//if(JS_ToInt64(ctx, &valuepoint, argv[i - 1]) < 0) {
				//	throw std::runtime_error("NativePoint.call(NativeTypes::Pointer)解析失败");
				//}
				//dcArgPointer(thi->m_vm, (void*)valuepoint);

				nativePointClass* thii = (nativePointClass*)JS_GetOpaque(argv[i - 1], nativePointClass::id);
				dcArgPointer(thi->m_vm, (thii == 0) ? 0 : (void*)(thii->get()));
			}
			break;
			default:
				break;
			}
		}

		// 调用
		switch(thi->m_agreeOn[0]) {
		case NativeTypes::Bool:
		{
			auto v = dcCallBool(thi->m_vm, ori);
			ret = JS_NewBool(ctx, v);
		}
		break;
		case NativeTypes::Char:
		case NativeTypes::UnsignedChar:
		{
			auto v = dcCallChar(thi->m_vm, ori);
			ret = JS_NewInt32(ctx, v);
		}
		break;
		case NativeTypes::Short:
		case NativeTypes::UnsignedShort:
		{
			auto v = dcCallShort(thi->m_vm, ori);
			ret = JS_NewInt32(ctx, v);
		}
		break;
		case NativeTypes::Int:
		case NativeTypes::UnsignedInt:
		{
			auto v = dcCallInt(thi->m_vm, ori);
			ret = JS_NewInt32(ctx, v);
		}
		break;
		case NativeTypes::Long:
		case NativeTypes::UnsignedLong:
		{
			auto v = dcCallLong(thi->m_vm, ori);
			ret = JS_NewInt32(ctx, v);
		}
		break;
		case NativeTypes::LongLong:
		case NativeTypes::UnsignedLongLong:
		{
			auto v = dcCallLongLong(thi->m_vm, ori);
			ret = JS_NewInt64(ctx, v);
		}
		break;
		case NativeTypes::Float:
		{
			auto v = dcCallFloat(thi->m_vm, ori);
			ret = JS_NewFloat64(ctx, v);
		}
		break;
		case NativeTypes::Double:
		{
			auto v = dcCallDouble(thi->m_vm, ori);
			ret = JS_NewFloat64(ctx, v);
		}
		break;
		case NativeTypes::Pointer:
		{
			auto v = dcCallPointer(thi->m_vm, ori);
			ret = nativePointClass::newNativePoint((uintptr_t)v);
		}
		break;
		case NativeTypes::Void:
		{
			dcCallVoid(thi->m_vm, ori);
			ret = JS_UNDEFINED;
		}
		break;
		default:
			break;
		}
		dcReset(thi->m_vm);
	}
	catch(std::runtime_error& re) {
		spdlog::error(re.what());
		spdlog::error("错误发生在-函数：{}，文件：{}", __FUNCTION__, __FILE__);
		throw re;
	}
	catch(std::exception& e) {
		spdlog::error(e.what());
		spdlog::error("错误发生在-函数：{}，文件：{}", __FUNCTION__, __FILE__);
		throw e;
	}


	return ret;
}

JSValue nativePointClass::toNumber(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
	nativePointClass* thi = (nativePointClass*)JS_GetOpaque(newTarget, id);
	return JS_NewInt64(ctx, thi->m_ptr);
}
