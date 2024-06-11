#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <chrono>
#include <iomanip>

#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

using boost::asio::ip::tcp;

struct HttpResponseHeader {
    std::string filename;
    std::size_t contentLength = 0;
    std::string contentType; // 添加contentType成员变量
    bool acceptRanges = false;
};

 
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
        // 解析响应头
        HttpResponseHeader header = parseHttpResponseHeaders(response, path);

        // 调用saveToFile函数保存文件，传入contentLength
        // if(header.acceptRanges){
            // saveToFile(response, header.filename, header.contentLength,8);
        // }else
            saveToFile(response, header.filename, header.contentLength);

    }

private:
    tcp::resolver resolver_;
    tcp::socket socket_;
    boost::system::error_code error;
    std::string server_;
    std::string path_;

    HttpResponseHeader parseHttpResponseHeaders(boost::asio::streambuf& response, const std::string& path) {
        std::istream response_stream(&response);
        std::string header;
        HttpResponseHeader result;
        std::regex contentDispositionRegex("Content-Disposition:.*filename=\"([^\"]*)\"");
        std::regex contentLengthRegex("Content-Length: (\\d+)");
        std::regex contentTypeRegex("Content-Type: (.+)");
        std::regex acceptRangesRegex("Accept-Ranges: (\\w+)");
        std::smatch match;

        while (std::getline(response_stream, header) && header != "\r") {
            if (std::regex_search(header, match, contentDispositionRegex) && match.size() > 1) {
                result.filename = match[1];
            } else if (std::regex_search(header, match, contentLengthRegex) && match.size() > 1) {
                result.contentLength = std::stoul(match[1].str());
            } else if (std::regex_search(header, match, contentTypeRegex) && match.size() > 1) {
                result.contentType = match[1];
            } else if (std::regex_search(header, match, acceptRangesRegex) && match.size() > 1) {
                result.acceptRanges = (match[1] == "bytes");
            }
        }

        if (result.filename.empty()) {
            // Fallback to extracting filename from URL if not found in headers
            std::size_t lastSlashPos = path.find_last_of('/');
            if (lastSlashPos != std::string::npos) {
                result.filename = path.substr(lastSlashPos + 1);
                if (result.filename.empty()) {
                    result.filename = "default_filename.html";
                }
            }
        }

        return result;
    }

    void saveToFile(boost::asio::streambuf& response, const std::string& filename, std::size_t contentLength) {
        std::ofstream outputFile(filename, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        std::size_t totalBytesWritten = 0;
        auto startTime = std::chrono::steady_clock::now();

        // 首先，将已经读取到response中的数据写入文件
        std::size_t bytesWritten = response.size();
        outputFile << &response;
        totalBytesWritten += bytesWritten;

        // 显示初始进度
        std::cout << "Downloaded 0% \r";
        std::cout.flush();

        while (boost::asio::read(socket_, response, boost::asio::transfer_at_least(1), error)) {
            bytesWritten = response.size();
            outputFile << &response;
            totalBytesWritten += bytesWritten;

            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            double speed = elapsedTime > 0 ? (totalBytesWritten / 1024.0) / elapsedTime : 0; // KB/s

            int progress = static_cast<int>(100.0 * totalBytesWritten / contentLength);
            
            double remainingBytes = contentLength - totalBytesWritten;
            int remainingSeconds = speed > 0 ? static_cast<int>(remainingBytes / speed / 1024) : 0;

            int hours = remainingSeconds / 3600;
            int minutes = (remainingSeconds % 3600) / 60;
            int seconds = remainingSeconds % 60;


            // std::cout << "Downloaded " << progress << "% (" << totalBytesWritten / 1024 << " KB, " << speed << " KB/s) \r";
            std::cout << "\r";
            std::cout << "Downloaded: " << std::setw(3) << std::setfill('-') << progress << "% ";
            std::cout << "[" << std::setw(6) << std::fixed << std::setprecision(2) << speed << " KB/s] ";
            std::cout << "[ETA: " << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds << "]";
            std::cout.flush();
        }

        // std::cout << std::endl; // 为了清晰，下载完成后换行
        auto endTime = std::chrono::steady_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
        double averageSpeed = totalBytesWritten / 1024.0 / totalTime;

        std::cout << "\r" << std::string(60, ' ') << "\r"; // 清空实时速度打印
        std::cout << "File size: " << (contentLength / 1024.0) << " KB" << std::endl;
        std::cout << "Average download speed: " << std::fixed << std::setprecision(2) << averageSpeed << " KB/s" << std::endl;
        std::cout << "Total time: " << totalTime << " seconds" << std::endl;

        if (error != boost::asio::error::eof) {
            std::cerr << "Error during receive: " << error.message() << std::endl;
        }

        outputFile.close();
    }

    
    void saveToFile(boost::asio::streambuf& response, const std::string& filename, std::size_t contentLength, std::size_t numThreads) {
        // 创建一个数组，用来实时写入各个进程的进度
        std::vector<std::size_t> progress(numThreads);

    }

};
