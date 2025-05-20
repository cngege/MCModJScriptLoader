#include "assets_net.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "http/httplib.h"

#include "spdlog/spdlog.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "imgui/imgui.h"

namespace fs = std::filesystem;

void http::downFont_JNMYT(std::filesystem::path fontsDir) {
	// httplib 下载字体文件
	if(!fs::exists(fontsDir / "JNMYT.ttf")) {
		httplib::Client fontdown("https://github.com");
		fontdown.enable_server_certificate_verification(false);
		fontdown.set_follow_location(true);
		auto res = fontdown.Get("/cngege/MCModJScriptLoader/releases/download/0.0.1/JNMYT1.2.ttf", [](uint64_t len, uint64_t total) {
			spdlog::debug("{}% ==>  {} / {}", (int)(len * 100 / total), len, total);
			return true;
		});
		if(res->status == httplib::StatusCode::OK_200) {
			std::ofstream font(fontsDir / "JNMYT.ttf", std::ios::binary);
			if(font.is_open()) {
				font.write(res->body.data(), res->body.size());
				font.close();
				spdlog::info("字体文件已经下载: {}, size {}", "JNMYT.ttf", res->body.size());

				// 装备至ImGui
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				fs::path font_JNMYT = fontsDir / "JNMYT.ttf";
				if(fs::exists(font_JNMYT)) {
					io.Fonts->AddFontFromFileTTF(font_JNMYT.string().c_str(), 14.5f, NULL, io.Fonts->GetGlyphRangesChineseFull());
				}
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
