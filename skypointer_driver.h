#pragma once

#include <cstdint>

#define N_CALIB_REGS 3
#define STEPS_PER_REV 3200

typedef struct {
    float z[N_CALIB_REGS];
} skypointerCalib;

bool skypointer_goto(int fd, int az, int alt, int speed);
bool skypointer_move(int fd, int az, int alt, int speed);
bool skypointer_stop(int fd);
bool skypointer_home(int fd);
bool skypointer_quit(int fd);

bool set_skypointer_laser(int fd, bool enable);
bool set_skypointer_timeout(int fd, int millis);
bool get_skypointer_pos(int fd, int * az, int * alt);
bool get_skypointer_version(int fd, char * version);

bool get_skypointer_calib(int fd, skypointerCalib *calib);
bool set_skypointer_calib(int fd, skypointerCalib *calib);
