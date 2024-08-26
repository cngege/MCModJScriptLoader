#include "assets_net.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "http/httplib.h"

#include "spdlog/spdlog.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

void http::downFont_JNMYT(std::filesystem::path fontsDir) {
	// httplib 下载字体文件
	if(!fs::exists(fontsDir / "JNMYT.ttf")) {
		httplib::Client fontdown("https://ghproxy.cc");
		auto res = fontdown.Get("/https://github.com/cngege/MCModJScriptLoader/releases/download/0.0.1/JNMYT.ttf");
		if(res->status == httplib::StatusCode::OK_200) {
			std::ofstream font(fontsDir / "JNMYT.ttf", std::ios::binary);
			if(font.is_open()) {
				font.write(res->body.data(), res->body.size());
				font.close();
				spdlog::info("字体文件已经下载: {}, size {}", "JNMYT.ttf", res->body.size());
			}
			else {
				spdlog::warn("字体文件已经下载完成，但在本地创建文件时失败");
			}
		}
		else {
			spdlog::warn("字体文件下载失败：status {}", res->status);
		}
	}
}
