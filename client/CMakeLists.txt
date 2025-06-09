add_subdirectory(mem)
add_subdirectory(utils)

target_sources(MCModJScriptLoader PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/ModManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ConfigManager.cpp
)