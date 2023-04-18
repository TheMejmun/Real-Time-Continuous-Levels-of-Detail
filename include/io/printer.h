//
// Created by Sam on 2023-04-11.
//

#ifndef REALTIME_CELL_COLLAPSE_PRINTER_H
#define REALTIME_CELL_COLLAPSE_PRINTER_H

#include <iostream>
#include <stdexcept>

//#define INFO_PRINTING
#define DEBUG_PRINTING
//#define VERBOSE_PRINTING
#define FPS_PRINTING

#define COUT std::cout <<
#define ENDL << std::endl
#define THROW(message) throw std::runtime_error(message)

#if(defined(NDEBUG) || !defined(INFO_PRINTING))
#define INF if(false) COUT
#else
#define INF COUT
#endif

#if(defined(NDEBUG) || !defined(DEBUG_PRINTING))
#define DBG if(false) COUT
#else
#define DBG COUT
#endif

#if(defined(NDEBUG) || !defined(VERBOSE_PRINTING))
#define VRB if(false) COUT
#else
#define VRB COUT
#endif

#if(defined(NDEBUG) || !defined(FPS_PRINTING))
#define FPS if(false) COUT
#else
#define FPS COUT
#endif

#define START_TRACE auto _trace_before = Timer::now();
#define END_TRACE(name) { \
    auto _trace_after = Timer::now(); \
    auto _duration = std::to_string(Timer::duration(_trace_before, _trace_after)); \
    TRC name << ": " << _duration ENDL; \
}


#endif //REALTIME_CELL_COLLAPSE_PRINTER_H
