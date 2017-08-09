#include "indicom.h"
#include "skypointer.h"
#include "skypointer_driver.h"

#include <libnova/sidereal_time.h>
#include <libnova/julian_day.h>

#include <cmath>
#include <memory>

//const float SIDE_RATE = 0.004178; /* sidereal rate, degrees/s */
const int SLEW_RATE = 1;        /* slew rate, degrees/s */
const int POLLMS    = 250;      /* poll period, ms */


std::unique_ptr<SkyPointer> skyPointer(new SkyPointer());


void ISGetProperties(const char * dev)
{
    skyPointer->ISGetProperties(dev);
}

void ISNewSwitch(const char * dev, const char * name, ISState * states, char * names[], int n)
{
    skyPointer->ISNewSwitch(dev, name, states, names, n);
}

void ISNewText(const char * dev, const char * name, char * texts[], char * names[], int n)
{
    skyPointer->ISNewText(dev, name, texts, names, n);
}

void ISNewNumber(const char * dev, const char * name, double values[], char * names[], int n)
{
    skyPointer->ISNewNumber(dev, name, values, names, n);
}

void ISNewBLOB(const char * dev, const char * name, int sizes[], int blobsizes[], char * blobs[], char * formats[],
               char * names[], int n)
{
    INDI_UNUSED(dev);
    INDI_UNUSED(name);
    INDI_UNUSED(sizes);
    INDI_UNUSED(blobsizes);
    INDI_UNUSED(blobs);
    INDI_UNUSED(formats);
    INDI_UNUSED(names);
    INDI_UNUSED(n);
}

void ISSnoopDevice(XMLEle * root)
{
    INDI_UNUSED(root);
}

SkyPointer::SkyPointer()
{
    //currentRA  = ln_get_apparent_sidereal_time(ln_get_julian_from_sys());
    currentRA  = 0;
    currentDEC = 90;

    // We add an additional debug level so we can log verbose scope status
    DBG_SCOPE = INDI::Logger::getInstance().addDebugLevel("Scope Verbose", "SCOPE");

    SetTelescopeCapability(TELESCOPE_CAN_SYNC |TELESCOPE_CAN_GOTO | TELESCOPE_CAN_ABORT);
}

bool SkyPointer::initProperties()
{
    INDI::Telescope::initProperties();

    IUFillText(&FirmwareT[0], "Version", "", 0);
    IUFillTextVector(&FirmwareTP, FirmwareT, 1, getDeviceName(),
                     "Firmware Info", "", "Device Info", IP_RO, 0, IPS_IDLE);

    addDebugControl();

    return true;
}

void SkyPointer::ISGetProperties(const char * dev)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) != 0)
        return;

    INDI::Telescope::ISGetProperties(dev);

    if (isConnected())
    {
        defineText(&FirmwareTP);
    }
}

bool SkyPointer::updateProperties()
{
    char version[16];

    if (isConnected())
    {
        if (get_skypointer_version(PortFD, version))
        {
            IUSaveText(&FirmwareT[0], version);
            defineText(&FirmwareTP);
        }
        else
        {
            DEBUG(INDI::Logger::DBG_WARNING, "Failed to retrive firmware information.");
            return false;
        }
        INDI::Telescope::updateProperties();
    }
    else
    {
        INDI::Telescope::updateProperties();
        deleteProperty(FirmwareTP.name);
    }
    return true;
}


const char * SkyPointer::getDefaultName()
{
    return "SkyPointer";
}

bool SkyPointer::Goto(double ra, double dec)
{
    targetRA  = ra;
    targetDEC = dec;
    char RAStr[64]= {0}, DecStr[64]= {0};

    // Parse the RA/DEC into strings
    fs_sexa(RAStr, targetRA, 2, 3600);
    fs_sexa(DecStr, targetDEC, 2, 3600);

    // Mark state as slewing
    TrackState = SCOPE_SLEWING;

    // Inform client we are slewing to a new position
    DEBUGF(INDI::Logger::DBG_SESSION, "Slewing to RA: %s - DEC: %s", RAStr, DecStr);

    // Success!
    return true;
}

bool SkyPointer::Sync(double ra, double dec)
{
    INDI_UNUSED(ra);
    INDI_UNUSED(dec);
    // Success!
    return true;
}

bool SkyPointer::Abort()
{
    return true;
}

bool SkyPointer::ReadScopeStatus()
{
    static struct timeval ltv
    {
        0, 0
    };
    struct timeval tv
    {
        0, 0
    };
    double dt = 0, da_ra = 0, da_dec = 0, dx = 0, dy = 0;
    int nlocked;

    /* update elapsed time since last poll, don't presume exactly POLLMS */
    gettimeofday(&tv, nullptr);

    if (ltv.tv_sec == 0 && ltv.tv_usec == 0)
        ltv = tv;

    dt  = tv.tv_sec - ltv.tv_sec + (tv.tv_usec - ltv.tv_usec) / 1e6;
    ltv = tv;

    // Calculate how much we moved since last time
    da_ra  = SLEW_RATE * dt;
    da_dec = SLEW_RATE * dt;

    /* Process per current state. We check the state of EQUATORIAL_EOD_COORDS_REQUEST and act acoordingly */
    switch (TrackState)
    {
        case SCOPE_SLEWING:
            // Wait until we are "locked" into positon for both RA & DEC axis
            nlocked = 0;

            // Calculate diff in RA
            dx = targetRA - currentRA;

            // If diff is very small, i.e. smaller than how much we changed since last time, then we reached target RA.
            if (fabs(dx) * 15. <= da_ra)
            {
                currentRA = targetRA;
                nlocked++;
            }
            // Otherwise, increase RA
            else if (dx > 0)
                currentRA += da_ra / 15.;
            // Otherwise, decrease RA
            else
                currentRA -= da_ra / 15.;

            // Calculate diff in DEC
            dy = targetDEC - currentDEC;

            // If diff is very small, i.e. smaller than how much we changed since last time, then we reached target DEC.
            if (fabs(dy) <= da_dec)
            {
                currentDEC = targetDEC;
                nlocked++;
            }
            // Otherwise, increase DEC
            else if (dy > 0)
                currentDEC += da_dec;
            // Otherwise, decrease DEC
            else
                currentDEC -= da_dec;

            // Let's check if we recahed position for both RA/DEC
            if (nlocked == 2)
            {
                // Let's set state to TRACKING
                TrackState = SCOPE_TRACKING;

                DEBUG(INDI::Logger::DBG_SESSION, "Telescope slew is complete. Tracking...");
            }
            break;

        default:
            break;
    }

    char RAStr[64]= {0}, DecStr[64]= {0};

    // Parse the RA/DEC into strings
    fs_sexa(RAStr, currentRA, 2, 3600);
    fs_sexa(DecStr, currentDEC, 2, 3600);

    DEBUGF(DBG_SCOPE, "Current RA: %s Current DEC: %s", RAStr, DecStr);

    NewRaDec(currentRA, currentDEC);
    return true;
}
