#include <boost/asio.hpp>
#include "downloader.hpp"
#include <iostream>
#include <string>
#include <regex>

int main() {
    try {
        // 获取URL
        std::string url;
        std::cout << "Enter URL to download: ";
        std::getline(std::cin, url);

        // 校验URL是否合法
        std::regex url_regex("^(http|https)://([^/]+)(/.*)?$");
        std::smatch url_match_result;
        if (!std::regex_match(url, url_match_result, url_regex)) {
            std::cerr << "Invalid URL\n";
            return -1;
        }

        // 使用正则表达式匹配结果
        std::string protocol = url_match_result[1];
        std::string server = url_match_result[2];
        std::string path = url_match_result[3].matched ? url_match_result[3] : std::string("/");


        std::cout << "Protocol: " << protocol << "\n";
        std::cout << "Server: " << server << "\n";
        std::cout << "Path: " << path << "\n";


        // 创建一个IO上下文
        boost::asio::io_context io_context;
        // 实例化一个下载器对象
        Downloader downloader(io_context, server, path);
        // 启动IO上下文
        io_context.run();
        
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}