#pragma once

#include "inditelescope.h"

class SkyPointer : public INDI::Telescope
{
    public:
        SkyPointer();
        virtual void ISGetProperties(const char * dev);

    protected:
        const char * getDefaultName();
        bool initProperties();
        bool updateProperties();

        // Telescope specific functions
        bool ReadScopeStatus();
        bool Goto(double, double);
        bool Sync(double, double);
        bool Abort();

        IText FirmwareT[1];
        ITextVectorProperty FirmwareTP;

    private:
        double currentRA;
        double currentDEC;
        double targetRA;
        double targetDEC;

        unsigned int DBG_SCOPE;
};
