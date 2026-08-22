#pragma once

#include "Services/Estimation/SWC_EKFAHRS.h"

// Phase 1 extraction facade. The full legacy EKF remains in Submodules/EKF for
// flight firmware; this portable facade exposes the same SWC-facing contract.
using AttitudeEKF_Extract = SWC_EKFAHRS;
