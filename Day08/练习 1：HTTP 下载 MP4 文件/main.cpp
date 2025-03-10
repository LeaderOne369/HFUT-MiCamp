#include "get.h"
#include "download.h"
#include <iostream>

int main(int argc, char *argv[]) 
{
    std::string url = "http://www.example.com";
    std::string mp4_url = "http://tianyue.oss-cn-hangzhou.aliyuncs.com/wukongmusic.mp4";
    
    std::cout << "测试HTTP GET请求..." << std::endl;
    int get_result = get(url);
    if (get_result == 0) {
        std::cout << "GET请求成功，结果保存在temp.html" << std::endl;
    } else {
        std::cout << "GET请求失败" << std::endl;
    }
    
    std::cout << "\n测试HTTP下载功能..." << std::endl;
    std::cout << "正在下载MP4文件: " << mp4_url << std::endl;
    int download_result = download(mp4_url);
    if (download_result == 0) {
        std::cout << "MP4文件下载成功!" << std::endl;
    } else {
        std::cout << "MP4文件下载失败" << std::endl;
    }
    
    return 0;
}
