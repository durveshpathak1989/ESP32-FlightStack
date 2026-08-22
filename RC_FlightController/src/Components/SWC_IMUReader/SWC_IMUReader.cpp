#include "SWC_IMUReader.h"

namespace {

constexpr float kGravity = 9.80665f;
constexpr float kDegToRad = flight::FlightConstants::kDegToRad;

}  // namespace

SWC_IMUReaderOutput SWC_IMUReader::update(const SWC_IMUReaderInput& input)
{
    SWC_IMUReaderOutput out;
    out.timestampUs = input.timestampUs;
    out.imu.ax = input.axG * kGravity;
    out.imu.ay = input.ayG * kGravity;
    out.imu.az = input.azG * kGravity;
    out.imu.gx = input.gxDps * kDegToRad;
    out.imu.gy = input.gyDps * kDegToRad;
    out.imu.gz = input.gzDps * kDegToRad;
    out.imu.mx = input.mx;
    out.imu.my = input.my;
    out.imu.mz = input.mz;
    out.imu.magValid = input.magValid;
    out.valid = input.accelValid && input.gyroValid;
    return out;
}
