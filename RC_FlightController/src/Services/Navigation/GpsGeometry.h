#pragma once

#include <cmath>

namespace flight {

struct GpsOffset {
    float northM = 0.0f;
    float eastM = 0.0f;
    float distanceM = 0.0f;
    float bearingRad = 0.0f;
};

inline float gpsDegToRad(double deg)
{
    return static_cast<float>(deg * 0.017453292519943295);
}

inline GpsOffset gpsOffsetMeters(double originLatDeg,
                                 double originLonDeg,
                                 double targetLatDeg,
                                 double targetLonDeg)
{
    constexpr float earthRadiusM = 6371000.0f;
    const float lat1 = gpsDegToRad(originLatDeg);
    const float lat2 = gpsDegToRad(targetLatDeg);
    const float dLat = gpsDegToRad(targetLatDeg - originLatDeg);
    const float dLon = gpsDegToRad(targetLonDeg - originLonDeg);
    const float meanLat = 0.5f * (lat1 + lat2);

    GpsOffset out;
    out.northM = earthRadiusM * dLat;
    out.eastM = earthRadiusM * std::cos(meanLat) * dLon;
    out.distanceM = std::sqrt(out.northM * out.northM + out.eastM * out.eastM);
    out.bearingRad = std::atan2(out.eastM, out.northM);
    return out;
}

}  // namespace flight
