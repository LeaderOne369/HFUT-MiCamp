#include "get.h"
#include <stdio.h>

int get(std::string url, char **rsp) {
    char *uri = const_cast<char*>(url.c_str());
    ghttp_request *request = NULL;
    ghttp_status status;
    int bytes_read;
    int size;

    request = ghttp_request_new();
    if (ghttp_set_uri(request, uri) == -1)
        return -1;
    if (ghttp_set_type(request, ghttp_type_get) == -1)
        return -1;
    ghttp_prepare(request);
    status = ghttp_process(request);
    if (status == ghttp_error)
        return -1;
    printf("Status code -> %d\n", ghttp_status_code(request));
    char* buf = ghttp_get_body(request);

    bytes_read = ghttp_get_body_len(request);
    size = strlen(buf);
    *rsp = new char[size + 1] { 0 };
    memcpy(*rsp, buf, size);

    ghttp_request_destroy(request);
    return 0;
}