#include "skypointer.h"
#include "config.h"

#include <cmath>
#include <memory>

#include <indicom.h>
#include <skypointer_driver.h>

using namespace INDI::AlignmentSubsystem;

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
    skyPointer->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

void ISSnoopDevice(XMLEle * root)
{
    skyPointer->ISSnoopDevice(root);
}

SkyPointer::SkyPointer()
{
    // We add an additional debug level so we can log verbose scope status
    DBG_SP = INDI::Logger::getInstance().addDebugLevel("SkyPointer Verbose", "SKYPOINTER");

    SetTelescopeCapability(TELESCOPE_CAN_SYNC |TELESCOPE_CAN_GOTO | TELESCOPE_CAN_ABORT);
    setTelescopeConnection(CONNECTION_SERIAL);
}

bool SkyPointer::initProperties()
{
    INDI::Telescope::initProperties();

    ScopeParametersN[0].value = 10;     // diameter
    ScopeParametersN[1].value = 100;    // focal length
    ScopeParametersN[2].value = 10;     // guide scope diameter
    ScopeParametersN[3].value = 100;    // guide scope focal length

    IUFillText(&FirmwareT[0], "Version", "", 0);
    IUFillTextVector(&FirmwareTP, FirmwareT, 1, getDeviceName(), "Firmware Info",
            "", "Device Info", IP_RO, 0, IPS_IDLE);

    IUFillSwitch(&LaserS[0], "On", "", ISS_OFF);
    IUFillSwitch(&LaserS[1], "Off", "", ISS_OFF);
    IUFillSwitchVector(&LaserSP, LaserS, 2, getDeviceName(), "Laser", "",
            MAIN_CONTROL_TAB, IP_WO, ISR_1OFMANY, 0, IPS_IDLE);

    addDebugControl();

    // Add alignment properties
    InitAlignmentProperties(this);

    return true;
}

bool SkyPointer::Connect()
{
    if (!INDI::Telescope::Connect())
        return false;

    if (!get_skypointer_version(PortFD, fw_version)) {
        DEBUG(INDI::Logger::DBG_ERROR, "Failed to retrive firmware information.");
        return false;
    }
    if (!skypointer_home(PortFD)) {
        DEBUG(INDI::Logger::DBG_ERROR, "Failed to home SkyPointer.");
        return false;
    }
    //SetTimer(POLLMS);

    return true;
}

bool SkyPointer::Disconnect()
{
    if (!skypointer_quit(PortFD)) {
        DEBUG(INDI::Logger::DBG_ERROR, "Failed to power SkyPointer off.");
        return false;
    }
    return INDI::Telescope::Disconnect();
}

bool SkyPointer::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
        ProcessAlignmentNumberProperties(this, name, values, names, n);

    return INDI::Telescope::ISNewNumber(dev, name, values, names, n);
}

bool SkyPointer::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        ProcessAlignmentSwitchProperties(this, name, states, names, n);

        if (strcmp(name, LaserSP.name) == 0)
        {
            IUUpdateSwitch(&LaserSP, states, names, n);
            set_skypointer_laser(PortFD, (LaserS[0].s == ISS_ON));
            LaserSP.s = IPS_OK;
            IDSetSwitch(&LaserSP, nullptr);
            return true;
        }
    }

    return INDI::Telescope::ISNewSwitch(dev, name, states, names, n);
}

bool SkyPointer::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
        ProcessAlignmentTextProperties(this, name, texts, names, n);

    return INDI::Telescope::ISNewText(dev, name, texts, names, n);
}

bool SkyPointer::ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[],
                         char *formats[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
        ProcessAlignmentBLOBProperties(this, name, sizes, blobsizes, blobs, formats, names, n);

    return INDI::Telescope::ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

bool SkyPointer::updateProperties()
{
    if (isConnected())
    {
        IUSaveText(&FirmwareT[0], fw_version);
        defineText(&FirmwareTP);
        defineSwitch(&LaserSP);
        INDI::Telescope::updateProperties();
    }
    else
    {
        INDI::Telescope::updateProperties();
        deleteProperty(FirmwareTP.name);
        deleteProperty(LaserSP.name);
    }
    return true;
}

const char * SkyPointer::getDefaultName()
{
    return "SkyPointer";
}

bool SkyPointer::MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command)
{
    int steps = (dir == DIRECTION_NORTH) ? STEPS_PER_REV/4 : -STEPS_PER_REV/4;
    int st = (command == MOTION_START);
    DEBUGF(DBG_SP, "Moving NS: steps=%d st=%d", steps, st);

    if (command == MOTION_START) {
        return skypointer_move(PortFD, 0, steps);
    }
    return skypointer_stop(PortFD);
}

bool SkyPointer::MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command)
{
    int steps = (dir == DIRECTION_WEST) ? STEPS_PER_REV : -STEPS_PER_REV;
    int st = (command == MOTION_START);
    DEBUGF(DBG_SP, "Moving WE: steps=%d st=%d", steps, st);

    if (command == MOTION_START) {
        return skypointer_move(PortFD, steps, 0);
    }
    return skypointer_stop(PortFD);
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
    DEBUGF(DBG_SP, "Slewing to RA: %s - DEC: %s", RAStr, DecStr);
    return true;
}

bool SkyPointer::Sync(double ra, double dec)
{
    // Parse the RA/DEC into strings
    char RAStr[64]={0}, DecStr[64]={0}, coordStr[64]={0};
    fs_sexa(RAStr, ra, 2, 3600);
    fs_sexa(DecStr, dec, 2, 3600);
    sprintf(coordStr, "RA:%s DEC:%s", RAStr, DecStr);

    int az, alt;
    if (!get_skypointer_pos(PortFD, &az, &alt)) {
        DEBUG(INDI::Logger::DBG_ERROR, "Failed to get skypointer position.");
    }
    struct ln_hrz_posn AltAz;
    AltAz.az = 360*double(az)/STEPS_PER_REV;
    AltAz.alt = 360*double(alt)/STEPS_PER_REV;

    AlignmentDatabaseEntry NewEntry;
    NewEntry.ObservationJulianDate = ln_get_julian_from_sys();
    NewEntry.RightAscension        = ra;
    NewEntry.Declination           = dec;
    NewEntry.TelescopeDirection    = TelescopeDirectionVectorFromAltitudeAzimuth(AltAz);
    NewEntry.PrivateDataSize       = 0;

    DEBUGF(DBG_SP, "Sync - Reference frame: %s", coordStr);

    if (CheckForDuplicateSyncPoint(NewEntry)) {
        DEBUGF(DBG_SP, "Sync - duplicate entry. %s", coordStr);
        return false;
    }

    GetAlignmentDatabase().push_back(NewEntry);

    // Tell the client about size change
    UpdateSize();

    // Tell the math plugin to reinitialise
    Initialise(this);
    DEBUGF(DBG_SP, "Sync - new entry added. %s", coordStr);
    ReadScopeStatus();
    return true;
}

bool SkyPointer::Abort()
{
        if (!skypointer_stop(PortFD)) {
            DEBUG(INDI::Logger::DBG_ERROR, "Failed to stop skypointer.");
            return false;
        }
    return true;
}

bool SkyPointer::ReadScopeStatus()
{
    ln_hrz_posn AltAz;
    int az, alt;

    if (!get_skypointer_pos(PortFD, &az, &alt)) {
        DEBUG(INDI::Logger::DBG_ERROR, "Failed to get skypointer position.");
    }
    AltAz.az = 360*double(az)/STEPS_PER_REV;
    AltAz.alt = 360*double(alt)/STEPS_PER_REV;
    TelescopeDirectionVector TDV = TelescopeDirectionVectorFromAltitudeAzimuth(AltAz);

    static struct timeval ltv { 0, 0 };
    struct timeval tv { 0, 0 };
    double dt = 0, da_ra = 0, da_dec = 0, dx = 0, dy = 0;
    int nlocked;

    /* update elapsed time since last poll, don't presume exactly POLLMS */
    gettimeofday(&tv, nullptr);

    if (ltv.tv_sec == 0 && ltv.tv_usec == 0)
        ltv = tv;

    dt  = tv.tv_sec - ltv.tv_sec + (tv.tv_usec - ltv.tv_usec) / 1e6;
    ltv = tv;

    //DEBUGF(DBG_SP, "ReadScopeStatus. dt=%f az=%f alt=%f", dt, AltAz.az, AltAz.alt);
    DEBUGF(DBG_SP, "MovementNSSP.s: %d MovementWESP.s: %d", MovementNSSP.s, MovementWESP.s);

    // Calculate how much we moved since last time
    da_ra  = SLEW_RATE * dt;
    da_dec = SLEW_RATE * dt;

    /* Process per current state. We check the state of EQUATORIAL_EOD_COORDS_REQUEST and act acoordingly */
    switch (TrackState) {
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

                DEBUG(DBG_SP, "Telescope slew is complete. Tracking...");
            }
            break;

        default:
            break;
    }

    char RAStr[64]= {0}, DecStr[64]= {0};

    // Parse the RA/DEC into strings
    fs_sexa(RAStr, currentRA, 2, 3600);
    fs_sexa(DecStr, currentDEC, 2, 3600);

    DEBUGF(DBG_SP, "Current RA: %s Current DEC: %s", RAStr, DecStr);

    NewRaDec(currentRA, currentDEC);
    return true;
}

bool SkyPointer::updateLocation(double latitude, double longitude, double elevation)
{
    UpdateLocation(latitude, longitude, elevation);
    return true;
}
