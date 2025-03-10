// 今天作业一老师后面给的可以跑的代码：
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    Display *x11Display = XOpenDisplay(NULL);
    if (x11Display == NULL)
    {
        fprintf(stderr, "Failed to open X11 display\n");
        return 1;
    }

    int screen = DefaultScreen(x11Display);

    Window window = XCreateSimpleWindow(x11Display, RootWindow(x11Display, screen),
                                        0, 0, 800, 600, 0, BlackPixel(x11Display, screen),
                                        WhitePixel(x11Display, screen));
    XSelectInput(x11Display, window, ExposureMask | KeyPressMask);
    XMapWindow(x11Display, window);

    XEvent event;
    while (1)
    {
        XNextEvent(x11Display, &event);
        if (event.type == Expose)
        {
        }
        if (event.type == KeyPress)
        {
            break;
        }
    }

    XDestroyWindow(x11Display, window);
    XCloseDisplay(x11Display);
    return 0;
}