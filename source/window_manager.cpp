//
// Created by Sam on 2023-04-08.
//

#include <iostream>
#include "window_manager.h"
#include "printer.h"

void WindowManager::create(const std::string& t) {
	INF "Creating WindowManager" ENDL;

	this->title = t;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	this->window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);
	this->monitor = glfwGetPrimaryMonitor();
	pollMonitorResolution();
}

void WindowManager::updateTitle(const std::string& t) {
	this->title = t;
	if (this->window != nullptr) {
		glfwSetWindowTitle(this->window, this->title.c_str());
	}
}

bool WindowManager::shouldClose() const {
	return glfwWindowShouldClose(this->window);
}

void WindowManager::close() const {
	glfwSetWindowShouldClose(this->window, GLFW_TRUE);
}

void WindowManager::toggleFullscreen() {
	this->isMaximized = !this->isMaximized;
	if (this->isMaximized) {
		pollWindowPosition(); // For restoring later
		glfwSetWindowMonitor(this->window, glfwGetPrimaryMonitor(), 0, 0, this->monitorParams->width, this->monitorParams->height, GLFW_DONT_CARE);
	}
	else {
		glfwSetWindowMonitor(this->window, nullptr,
			this->windowPosX, this->windowPosY,
			this->width, this->height, GLFW_DONT_CARE);
	}
}

void WindowManager::pollMonitorResolution() {
	this->monitorParams = glfwGetVideoMode(this->monitor);
	DBG "Got monitor info:" <<
		"\n\tWidth:\t" << this->monitorParams->width <<
		"\n\tHeight:\t" << this->monitorParams->height <<
		"\n\tRed Bits:\t" << this->monitorParams->redBits <<
		"\n\tGreen Bits:\t" << this->monitorParams->greenBits <<
		"\n\tBlue Bits:\t" << this->monitorParams->blueBits <<
		"\n\tHz:\t" << this->monitorParams->refreshRate ENDL;
}

void WindowManager::pollWindowPosition() {
	glfwGetWindowPos(this->window, &this->windowPosX, &this->windowPosY);
}

void WindowManager::destroy() const {
	INF "Destroying WindowManager" ENDL;

	glfwDestroyWindow(this->window);

	glfwTerminate();
}