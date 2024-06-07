#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <regex>

using boost::asio::ip::tcp;

class Downloader {
public:
    Downloader(boost::asio::io_context& io_context, const std::string& server, const std::string& path)
        : resolver_(io_context), socket_(io_context), server_(server), path_(path) {
        // 解析服务器地址
        auto endpoints = resolver_.resolve(server, "http");
        // 连接到服务器
        boost::asio::connect(socket_, endpoints);
        // 构建HTTP请求
        std::string request = "GET " + path + " HTTP/1.0\r\n";
        request += "Host: " + server + "\r\n";
        request += "Accept: */*\r\n";
        request += "Connection: close\r\n\r\n";
        // 发送HTTP请求
        boost::asio::write(socket_, boost::asio::buffer(request, request.length()));
        // 读取响应
        boost::asio::streambuf response;
        boost::asio::read_until(socket_, response, "\r\n\r\n");
        // 打印HTTP头部并获取文件名
        std::string filename = parseHeadersAndGetFilename(response);
        // 调用saveToFile函数保存文件
        saveToFile(response, filename);
    }

private:
    tcp::resolver resolver_;
    tcp::socket socket_;
    boost::system::error_code error;
    std::string server_;
    std::string path_;

    std::string parseHeadersAndGetFilename(boost::asio::streambuf& response) {
        std::istream response_stream(&response);
        std::string header;
        std::string filename = "default_filename.html";
        std::regex contentDispositionRegex("Content-Disposition:.*filename=\"([^\"]*)\"");
        std::smatch match;

        while (std::getline(response_stream, header) && header != "\r") {
            // 注释掉头部打印
            // std::cout << header << "\n";
            if (std::regex_search(header, match, contentDispositionRegex) && match.size() > 1) {
                filename = match[1];
                break; // Found the filename
            }
        }

        if (filename == "default_filename.html") {
            // Fallback to extracting filename from URL if not found in headers
            std::size_t lastSlashPos = path_.find_last_of('/');
            if (lastSlashPos != std::string::npos) {
                filename = path_.substr(lastSlashPos + 1);
                if (filename.empty()) {
                    filename = "default_filename.html";
                }
            }
        }

        return filename;
    }

    void saveToFile(boost::asio::streambuf& response, const std::string& filename) {
        std::ofstream outputFile(filename);
        if (outputFile) {
            // 将响应流剩余部分写入文件
            outputFile << &response;
            outputFile.close();
        }
    }
};