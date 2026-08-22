#include "SWC_GPSReceiver.h"

#include <cmath>

SWC_GPSReceiverOutput SWC_GPSReceiver::update(const SWC_GPSReceiverInput& input)
{
    SWC_GPSReceiverOutput out;
    out.timestampUs = input.timestampUs;
    out.gps.latitude = input.latitude;
    out.gps.longitude = input.longitude;
    out.gps.altitude = input.altitude;
    const float courseRad = input.courseDeg * flight::FlightConstants::kDegToRad;
    out.gps.velNorth = input.groundSpeedMps * std::cos(courseRad);
    out.gps.velEast = input.groundSpeedMps * std::sin(courseRad);
    out.gps.velDown = 0.0f;
    out.gps.numSatellites = input.satellites;
    out.gps.hdop = input.hdop;
    out.gps.fix = input.valid
        ? (input.satellites >= 5 ? flight::GpsState::GPS_3D : flight::GpsState::GPS_2D)
        : flight::GpsState::NO_FIX;
    out.gps.timestampUs = input.timestampUs;
    return out;
}
