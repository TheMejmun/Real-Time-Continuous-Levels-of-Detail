//
// Created by Saman on 01.09.23.
//

#include "io/printer.h"

#include <sstream>
#include <thread>

void Printer::printThreadStarted() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string id = ss.str();
    printf("Thread id: %s\n", id.c_str());
}