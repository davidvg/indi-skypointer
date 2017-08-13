
#pragma once

#include "skypointer_driver.h"

#include <inditelescope.h>
#include <alignment/AlignmentSubsystemForDrivers.h>

class SkyPointer : public INDI::Telescope, public INDI::AlignmentSubsystem::AlignmentSubsystemForDrivers
{
    public:
        SkyPointer();

    protected:
        bool Connect();
        bool Disconnect();
        const char * getDefaultName();
        bool initProperties();
        bool updateProperties();

        // Telescope specific functions
        bool ReadScopeStatus();
        bool Goto(double, double);
        bool Sync(double, double);
        bool MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command);
        bool MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command);
        bool Abort();

        IText FirmwareT[1];
        ITextVectorProperty FirmwareTP;
        ISwitch LaserS[2];
        ISwitchVectorProperty LaserSP;

    private:
        double currentRA;
        double currentDEC;
        double targetRA;
        double targetDEC;
        char fwVersion[16];
        uint8_t motorSpeed;
        skypointerCalib calib;

        friend void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n);
        virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
        friend void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n);
        virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
        friend void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n);
        virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
        friend void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n);
        virtual bool ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n) override;
        virtual bool updateLocation(double latitude, double longitude, double elevation) override;
        int updateMotorSpeed();

        unsigned int DBG_SP;
};
