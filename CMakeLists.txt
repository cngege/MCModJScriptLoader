# CMakeList.txt: MCModJScriptLoader 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)
set(REQUIRED_WINDOWS_SDK_VERSION 10.0.26100.0)
# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()
add_compile_options(/utf-8)

add_compile_options(/wd26498)


if(CMAKE_BUILD_TYPE MATCHES Release)
# release 下生成pdb
	MESSAGE(STATUS "PDB - 当前在 RELEASE 模式下 生成pdb文件")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
else()
	MESSAGE(STATUS "PDB - 当前在 DEBUG 模式下 生成完整pdb文件")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi")
endif()

#project ("MCModJScriptLoader")
include(FetchContent)
include_directories(include)

# 将源代码添加到此项目的可执行文件。
add_library(MCModJScriptLoader SHARED)
target_sources(MCModJScriptLoader PRIVATE MCModJScriptLoader.cpp)


add_subdirectory(client)
add_subdirectory(hook)
add_subdirectory(jsClass)
add_subdirectory(include)

set(GITPROXY_HEADER https://ghproxy.cn/https://)
if(DEFINED GitHubActions)
	set(LIBSSL_URL https://github.com/cngege/cngege.github.io/releases/download/v0.1-beta.1/libssl.lib)
	set(CRYPTO_URL https://github.com/cngege/cngege.github.io/releases/download/v0.1-beta.1/libcrypto.lib)
	set(WINRT_URL https://github.com/cngege/cngege.github.io/releases/download/v0.1-beta.2/cppwinrt_fast_forwarder-2.0.250303.1.lib)
else()
	set(LIBSSL_URL ${GITPROXY_HEADER}github.com/cngege/cngege.github.io/releases/download/v0.1-beta.1/libssl.lib)
	set(CRYPTO_URL ${GITPROXY_HEADER}github.com/cngege/cngege.github.io/releases/download/v0.1-beta.1/libcrypto.lib)
	set(WINRT_URL ${GITPROXY_HEADER}github.com/cngege/cngege.github.io/releases/download/v0.1-beta.2/cppwinrt_fast_forwarder-2.0.250303.1.lib)
endif()

# 新建openssl文件夹
FIND_FILE(LOCAL_OPENSSL_LIBRARIES_PATH
	NAMES ""
	HINTS "lib/openssl"
	NO_DEFAULT_PATH)
if(NOT EXISTS "lib/openssl")
	file(MAKE_DIRECTORY "lib/openssl")
	SET(LOCAL_OPENSSL_LIBRARIES_PATH "lib/openssl")
	MESSAGE(STATUS "Folder “lib/openssl” created successfully.")
endif()

# /////////////////////////////////////ssl库lib名称////////////////////////////////////////////////
SET(OPENSSL_SSL_NAME "libssl.lib")
# ssl远程下载地址
MESSAGE(STATUS "Checking ${OPENSSL_SSL_NAME}...")
# 判断库文件是否已经存在
FIND_FILE(LOCAL_OPENSSL_SSL_LIB
	NAMES ${OPENSSL_SSL_NAME}
	HINTS "${PROJECT_SOURCE_DIR}/lib/openssl"
	NO_DEFAULT_PATH)
MESSAGE(STATUS "Checking url link ${LIBSSL_URL} ...")
# 如果路径下没有目标文件
IF(NOT LOCAL_OPENSSL_SSL_LIB)
	MESSAGE(STATUS "Downloading ${OPENSSL_SSL_NAME} to ${LOCAL_OPENSSL_LIBRARIES_PATH} ...")
		# 从设定的 URL 地址下载相应的压缩包
	FILE(DOWNLOAD ${LIBSSL_URL}
		${LOCAL_OPENSSL_LIBRARIES_PATH}/${OPENSSL_SSL_NAME}
		TIMEOUT 300 CACHE STRING "Timeout in seconds when downloading ${OPENSSL_SSL_NAME}"
		STATUS ERR
		SHOW_PROGRESS)
	# 判断下载是否存在错误
	IF(ERR EQUAL 0)
		# 如果下载无错误则设置压缩包名称标记下载成功
		SET(LOCAL_OPENSSL_SSL_LIB "${LOCAL_OPENSSL_LIBRARIES_PATH}/${OPENSSL_SSL_NAME}")
	#下载错误输出相关的提示信息
	ELSE()
		FILE(REMOVE ${LOCAL_OPENSSL_LIBRARIES_PATH}/${OPENSSL_SSL_NAME})
		MESSAGE(STATUS "Download failed, error: ${ERR}")
		MESSAGE(FATAL_ERROR
		"You can try downloading ${LIBSSL_URL} manually"
		"using curl/wget or a similar tool")
	ENDIF()
ENDIF()
# 如果路径下存在压缩文件
IF(LOCAL_OPENSSL_SSL_LIB)
	MESSAGE(STATUS "Checking the direction of ${LOCAL_OPENSSL_LIBRARIES_PATH}/${OPENSSL_SSL_NAME}")
ENDIF()

# /////////////////////////////////////crypto库lib名称////////////////////////////////////////////////
SET(OPENSSL_CRYPTO_NAME "libcrypto.lib")
# ssl远程下载地址
MESSAGE(STATUS "Checking ${OPENSSL_CRYPTO_NAME}...")
# 判断库文件是否已经存在
FIND_FILE(LOCAL_OPENSSL_CRYPTO_LIB
	NAMES ${OPENSSL_CRYPTO_NAME}
	HINTS "${PROJECT_SOURCE_DIR}/lib/openssl"
	NO_DEFAULT_PATH)
MESSAGE(STATUS "Checking url link ${CRYPTO_URL} ...")
# 如果路径下没有目标文件
IF(NOT LOCAL_OPENSSL_CRYPTO_LIB)
	MESSAGE(STATUS "Downloading ${OPENSSL_CRYPTO_NAME} to ${LOCAL_OPENSSL_LIBRARIES_PATH} ...")
		# 从设定的 URL 地址下载相应的压缩包
	FILE(DOWNLOAD ${CRYPTO_URL}
		${LOCAL_OPENSSL_LIBRARIES_PATH}/${OPENSSL_CRYPTO_NAME}
		TIMEOUT 300 CACHE STRING "Timeout in seconds when downloading ${OPENSSL_CRYPTO_NAME}."
		STATUS ERR
		SHOW_PROGRESS)
	# 判断下载是否存在错误
	IF(ERR EQUAL 0)
		# 如果下载无错误则设置压缩包名称标记下载成功
		SET(LOCAL_OPENSSL_CRYPTO_LIB "${LOCAL_OPENSSL_LIBRARIES_PATH}/${OPENSSL_CRYPTO_NAME}")
	#下载错误输出相关的提示信息
	ELSE()
		FILE(REMOVE ${LOCAL_OPENSSL_LIBRARIES_PATH}/${OPENSSL_CRYPTO_NAME})
		MESSAGE(STATUS "Download failed, error: ${ERR}")
		MESSAGE(FATAL_ERROR
		"You can try downloading ${CRYPTO_URL} manually"
		"using curl/wget or a similar tool")
	ENDIF()
ENDIF()
# 如果路径下存在压缩文件
IF(LOCAL_OPENSSL_CRYPTO_LIB)
	MESSAGE(STATUS "Checking the direction of ${LOCAL_OPENSSL_LIBRARIES_PATH}/${OPENSSL_CRYPTO_NAME}")
ENDIF()

# 包含 openssl 的头文件
include_directories(include/openssl)


# 新建cppwinrt文件夹
FIND_FILE(LOCAL_WINRT_LIBRARIES_PATH
	NAMES ""
	HINTS "lib/cppwinrt"
	NO_DEFAULT_PATH)
if(NOT EXISTS "lib/cppwinrt")
	file(MAKE_DIRECTORY "lib/cppwinrt")
	SET(LOCAL_WINRT_LIBRARIES_PATH "lib/cppwinrt")
	MESSAGE(STATUS "Folder “lib/cppwinrt” created successfully.")
endif()


# /////////////////////////////////////winrt库lib名称////////////////////////////////////////////////
SET(WINRT_NAME "cppwinrt.lib")
# ssl远程下载地址
MESSAGE(STATUS "Checking ${WINRT_NAME}...")
# 判断库文件是否已经存在
FIND_FILE(LOCAL_WINRT_LIB
	NAMES ${WINRT_NAME}
	HINTS "${PROJECT_SOURCE_DIR}/lib/cppwinrt"
	NO_DEFAULT_PATH)
MESSAGE(STATUS "Checking url link ${WINRT_URL} ...")
# 如果路径下没有目标文件
IF(NOT LOCAL_WINRT_LIB)
	MESSAGE(STATUS "Downloading ${WINRT_NAME} to ${LOCAL_WINRT_LIBRARIES_PATH} ...")
		# 从设定的 URL 地址下载相应的压缩包
	FILE(DOWNLOAD ${WINRT_URL}
		${LOCAL_WINRT_LIBRARIES_PATH}/${WINRT_NAME}
		TIMEOUT 300 CACHE STRING "Timeout in seconds when downloading ${WINRT_NAME}."
		STATUS ERR
		SHOW_PROGRESS)
	# 判断下载是否存在错误
	IF(ERR EQUAL 0)
		# 如果下载无错误则设置压缩包名称标记下载成功
		SET(LOCAL_WINRT_LIB "${LOCAL_WINRT_LIBRARIES_PATH}/${WINRT_NAME}")
	#下载错误输出相关的提示信息
	ELSE()
		FILE(REMOVE ${LOCAL_WINRT_LIBRARIES_PATH}/${WINRT_NAME})
		MESSAGE(STATUS "Download failed, error: ${ERR}")
		MESSAGE(FATAL_ERROR
		"You can try downloading ${WINRT_URL} manually"
		"using curl/wget or a similar tool")
	ENDIF()
ENDIF()
# 如果路径下存在压缩文件
IF(LOCAL_WINRT_LIB)
	MESSAGE(STATUS "Checking the direction of ${LOCAL_WINRT_LIBRARIES_PATH}/${WINRT_NAME}")
ENDIF()



# 链接相关库  
target_link_libraries(MCModJScriptLoader PRIVATE
	${CMAKE_SOURCE_DIR}/lib/openssl/libssl.lib
	${CMAKE_SOURCE_DIR}/lib/openssl/libcrypto.lib
	${CMAKE_SOURCE_DIR}/lib/quickjs/quickjs-ng.lib
	${CMAKE_SOURCE_DIR}/lib/dyncall/dyncall_s.lib
	${CMAKE_SOURCE_DIR}/lib/dyncall/dyncallback_s.lib
	${CMAKE_SOURCE_DIR}/lib/dyncall/dynload_s.lib
	${CMAKE_SOURCE_DIR}/lib/cppwinrt/cppwinrt.lib
)
# cppwinrt_fast_forwarder.lib 来自"https://www.nuget.org/packages/Microsoft.Windows.CppWinRT/#usedby-body-tab" 右侧 Download package, 解压后lib

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MCModJScriptLoader PROPERTY CXX_STANDARD 20)
endif()


