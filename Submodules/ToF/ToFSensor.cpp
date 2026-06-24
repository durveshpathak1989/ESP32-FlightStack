#include "ToFSensor.h"
#include "DebugConfig.h"

ToFSensor tofSensor(Wire);

ToFSensor::ToFSensor(TwoWire& wire)
    : _wire(wire), _begun(false)
{
}

bool ToFSensor::begin(int sdaPin, int sclPin, uint32_t i2cHz)
{
    _wire.begin(sdaPin, sclPin, i2cHz);
    delay(10);

#if TOF_ENABLE_VL53L4CX
    if (!_vl53.begin(0x29, &_wire)) {
        DBG_PRINTLN(F("[TOF] VL53L4CX not found at 0x29."));
        _begun = false;
        return false;
    }
    _vl53.startRanging();
    _begun = true;
    DBG_PRINTF("[TOF] VL53L4CX OK on SDA=GPIO%d SCL=GPIO%d\n", sdaPin, sclPin);
    return true;
#else
    DBG_PRINTLN(F("[TOF] VL53L4CX wrapper present but disabled. Set TOF_ENABLE_VL53L4CX=1 after installing library."));
    _begun = false;
    return false;
#endif
}

bool ToFSensor::read(ToFData& out)
{
    out.valid = false;
    out.distance_m = 0.0f;
    out.ts_ms = millis();

#if TOF_ENABLE_VL53L4CX
    if (!_begun) return false;
    if (!_vl53.dataReady()) return false;

    VL53L4CX_MultiRangingData_t data;
    if (!_vl53.getRangingData(&data)) return false;
    _vl53.clearInterrupt();

    if (data.NumberOfObjectsFound < 1) return false;
    const int32_t mm = data.RangeData[0].RangeMilliMeter;
    if (mm <= 20 || mm > 4000) return false;

    out.distance_m = (float)mm * 0.001f;
    out.valid = true;
    out.ts_ms = millis();
    return true;
#else
    return false;
#endif
}
