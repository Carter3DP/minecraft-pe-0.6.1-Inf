#include "AppPlatform_glfw.h"

float AppPlatform_glfw::getPixelsPerMillimeter() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    int width_mm, height_mm;
    glfwGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    return (float)mode->width / (float)width_mm;
}

ScreenSafeBounds AppPlatform_glfw::getSafeZone() {
	ScreenSafeBounds margin;
    margin.left = 0;
    margin.right = getScreenWidth();
    margin.top = 0;
    margin.bottom = getScreenHeight();
    return margin;
}