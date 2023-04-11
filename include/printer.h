//
// Created by Sam on 2023-04-11.
//

#ifndef REALTIME_CELL_COLLAPSE_PRINTER_H
#define REALTIME_CELL_COLLAPSE_PRINTER_H

#include <string>
#include <sstream>
#include <iostream>

#define INFO_PRINTING
#define DEBUG_PRINTING
//#define TRACE_PRINTING

#define COUT std::cout <<
#define ENDL << std::endl

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

#if(defined(NDEBUG) || !defined(TRACE_PRINTING))
#define TRC if(false) COUT
#else
#define TRC COUT
#endif




#endif //REALTIME_CELL_COLLAPSE_PRINTER_H
