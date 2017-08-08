#pragma once

#include <inditelescope.h>

class SkyPointer : public INDI::Telescope
{
  public:
    SkyPointer();

  protected:
    // General device functions
    bool Connect();
    bool Disconnect();
    const char *getDefaultName();
    bool initProperties();

    // Telescope specific functions
    bool ReadScopeStatus();
    bool Goto(double, double);
    bool Abort();

  private:
    double currentRA;
    double currentDEC;
    double targetRA;
    double targetDEC;

    unsigned int DBG_SCOPE;
};
