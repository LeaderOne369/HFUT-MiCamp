#include "download.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "libghttp/ghttp.h"

int download(std::string downloadURL) {
    const char *url = downloadURL.c_str();
    ghttp_request *request = ghttp_request_new();
    ghttp_status status;

    std::cout << "开始下载文件: " << url << std::endl;

    if (ghttp_set_uri(request, (char*)url) == -1) {
        fprintf(stderr, "Error setting URI.\n");
        return -1;
    }

    if (ghttp_set_type(request, ghttp_type_get) == -1) {
        fprintf(stderr, "Error setting request type.\n");
        return -1;
    }

    // 设置一些HTTP头以处理大文件
    ghttp_set_header(request, http_hdr_Accept, "*/*");
    ghttp_set_header(request, http_hdr_Connection, "close");

    if (ghttp_prepare(request) == -1) {
        fprintf(stderr, "Error preparing request.\n");
        return -1;
    }

    std::cout << "正在发送请求..." << std::endl;
    status = ghttp_process(request);
    if (status == ghttp_error) {
        fprintf(stderr, "Error processing request: %s\n", ghttp_get_error(request));
        return -1;
    }

    // 获取HTTP状态码
    int status_code = ghttp_status_code(request);
    std::cout << "HTTP状态码: " << status_code << std::endl;
    
    if (status_code != 200) {
        fprintf(stderr, "下载失败，HTTP状态码: %d\n", status_code);
        return -1;
    }

    // 从URL中提取文件名
    std::string filename = "download.mp4";
    size_t last_slash = downloadURL.find_last_of("/");
    if (last_slash != std::string::npos && last_slash < downloadURL.length() - 1) {
        filename = downloadURL.substr(last_slash + 1);
    }

    std::cout << "将文件保存为: " << filename << std::endl;

    const char *body = ghttp_get_body(request);
    int body_len = ghttp_get_body_len(request);

    std::cout << "接收到数据大小: " << body_len << " 字节" << std::endl;

    FILE *file = fopen(filename.c_str(), "wb");
    if (!file) {
        fprintf(stderr, "Error opening file for writing.\n");
        return -1;
    }

    // 写入文件数据
    size_t written = fwrite(body, 1, body_len, file);
    if (written != (size_t)body_len) {
        fprintf(stderr, "Error writing to file: %zu of %d bytes written\n", written, body_len);
        fclose(file);
        return -1;
    }

    fclose(file);
    std::cout << "文件下载完成，已保存为: " << filename << std::endl;

    ghttp_request_destroy(request);

    return 0;
}