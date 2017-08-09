#include "skypointer_driver.h"

#include "indicom.h"
#include "indilogger.h"

#include <cstring>
#include <termios.h>
#include <unistd.h>

#define SKYPOINTER_TIMEOUT 2
#define MAX_MSG_SIZE 16

#if IS_BIG_ENDIAN
#error "This code works only in little-endian machines"
#endif

char sp_name[MAXINDIDEVICE] = "SkyPointer";


void set_skypointer_device(const char * name)
{
    strncpy(sp_name, name, MAXINDIDEVICE);
}

bool send_cmd(int fd, const char * cmd, char * response, int * response_len)
{
    int nbytes = 0;
    int err = 0;
    char errmsg[MAXRBUF];

    DEBUGFDEVICE(sp_name, INDI::Logger::DBG_DEBUG, "CMD (%s)", cmd);

    tcflush(fd, TCIOFLUSH);
    if ((err = tty_write(fd, cmd, strlen(cmd), &nbytes)) != TTY_OK)
    {
        tty_error_msg(err, errmsg, MAXRBUF);
        DEBUGFDEVICE(sp_name, INDI::Logger::DBG_ERROR, "%s", errmsg);
        return false;
    }
    if ((err = tty_read_section(fd, response, '\r', SKYPOINTER_TIMEOUT, response_len)))
    {
        tty_error_msg(err, errmsg, MAXRBUF);
        DEBUGFDEVICE(sp_name, INDI::Logger::DBG_ERROR, "%s", errmsg);
        return false;
    }

    if (*response_len > 0)
    {
        response[*response_len] = '\0';
        DEBUGFDEVICE(sp_name, INDI::Logger::DBG_DEBUG, "RES (%s)", response);
    }

    return true;
}

bool send_simple_cmd(int fd, const char * cmd)
{
    char resp[MAX_MSG_SIZE];
    int resp_len = 0;
    return send_cmd(fd, cmd, resp, &resp_len);
}

bool skypointer_goto(int fd, int az, int alt)
{
    char cmd[MAX_MSG_SIZE];
    sprintf(cmd, "G %d %d\r", az, alt);
    return send_simple_cmd(fd, (const char *)cmd);
}

bool skypointer_move(int fd, int az, int alt)
{
    char cmd[MAX_MSG_SIZE];
    sprintf(cmd, "M %d %d\r", az, alt);
    return send_simple_cmd(fd, (const char *)cmd);
}

bool skypointer_stop(int fd)
{
    char const * cmd = "S\r";
    return send_simple_cmd(fd, cmd);
}

bool skypointer_home(int fd)
{
    char const * cmd = "H\r";
    return send_simple_cmd(fd, cmd);
}

bool skypointer_quit(int fd)
{
    char const * cmd = "Q\r";
    return send_simple_cmd(fd, cmd);
}

bool set_skypointer_laser(int fd, bool enable)
{
    char cmd[MAX_MSG_SIZE];
    sprintf(cmd, "L %d\r", enable);
    return send_simple_cmd(fd, (const char *)cmd);
}

bool set_skypointer_timeout(int fd, int millis)
{
    char cmd[MAX_MSG_SIZE];
    sprintf(cmd, "T %d\r", millis);
    return send_simple_cmd(fd, (const char *)cmd);
}

bool get_skypointer_pos(int fd, int * az, int * alt)
{
    char const * cmd = "P\r";
    char resp[MAX_MSG_SIZE];
    int resp_len = 0;

    bool ret = send_cmd(fd, cmd, resp, &resp_len);
    if (ret)
    {
        sscanf(resp, "P %d %d", az, alt);
    }
    return ret;
}

bool get_skypointer_version(int fd, char * version)
{
    char const * cmd = "I\r";
    char resp[MAX_MSG_SIZE];
    int resp_len = 0;

    bool ret = send_cmd(fd, cmd, resp, &resp_len);
    if (ret)
    {
        sprintf(version, "%c.%c", resp[12], resp[14]);
    }
    return ret;
}

bool get_calib_reg(int fd, int n, float * val)
{
    char cmd[MAX_MSG_SIZE];
    char resp[MAX_MSG_SIZE];
    int resp_len = 0;
    int intval = 0;

    sprintf(cmd, "R %d\r", n);
    if (!send_cmd(fd, (const char *)cmd, resp, &resp_len))
    {
        return false;
    }

    sscanf(resp, "R %x", &intval);
    *val = *((float *) &intval);
    return true;
}

bool set_calib_reg(int fd, int n, float val)
{
    char cmd[MAX_MSG_SIZE];
    sprintf(cmd, "W %d %08x\r", n, *((int *) &val));
    return send_simple_cmd(fd, (const char *)cmd);
}

bool get_skypointer_calib(int fd, skypointerCalib * calib)
{
    for (int i=0; i<N_CALIB_REGS; i++)
    {
        if (!get_calib_reg(fd, i, &calib->z[i]))
            return false;
    }
    return true;
}

bool set_skypointer_calib(int fd, skypointerCalib * calib)
{
    for (int i=0; i<N_CALIB_REGS; i++)
    {
        if (!set_calib_reg(fd, i, calib->z[i]))
            return false;
    }
    return true;
}
