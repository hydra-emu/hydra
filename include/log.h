#pragma once

#include <stdio.h>

inline void log_warn(const char* message)
{
    printf("[WARN] %s\n", message);
}

inline void log_info(const char* message)
{
    printf("[INFO] %s\n", message);
}

inline void log_fatal(const char* message)
{
    printf("[ERROR] %s\n", message);
    exit(1);
}