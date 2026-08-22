CXX ?= g++
SRC_DIR := ../RC_FlightController/src
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Werror -I$(SRC_DIR)

COMMON_SRCS := \
	$(SRC_DIR)/Application/FlightApplication.cpp \
	$(SRC_DIR)/Application/ModeManager.cpp \
	$(SRC_DIR)/Components/SWC_BatteryMonitor/SWC_BatteryMonitor.cpp \
	$(SRC_DIR)/Components/SWC_FailsafeOrchestrator/SWC_FailsafeOrchestrator.cpp \
	$(SRC_DIR)/Components/SWC_GPSReceiver/SWC_GPSReceiver.cpp \
	$(SRC_DIR)/Components/SWC_IMUReader/SWC_IMUReader.cpp \
	$(SRC_DIR)/Components/SWC_MotorOutput/SWC_MotorOutput.cpp \
	$(SRC_DIR)/Components/SWC_PIDControl/SWC_PIDControl.cpp \
	$(SRC_DIR)/Components/SWC_RCReceiver/SWC_RCReceiver.cpp \
	$(SRC_DIR)/Components/SWC_Telemetry/SWC_Telemetry.cpp \
	$(SRC_DIR)/Services/Diagnostics/SWC_HealthMonitor.cpp \
	$(SRC_DIR)/Services/Estimation/AttitudeEKF_Extract.cpp \
	$(SRC_DIR)/Services/Estimation/Mahony_Extract.cpp \
	$(SRC_DIR)/Services/Estimation/SWC_EKFAHRS.cpp \
	$(SRC_DIR)/Services/Navigation/SWC_FailsafeLogic.cpp \
	$(SRC_DIR)/Services/Control/SWC_PIDController.cpp

TEST_BINS := \
	portable_core_compile_test \
	test_SWC_PIDController \
	test_SWC_EKFAHRS \
	test_FailsafeLogic \
	test_ControllerRegistry \
	test_ModeManager \
	test_FlightApplication

.PHONY: test_all test_portable clean

test_all: $(TEST_BINS)
	@for test in $(TEST_BINS); do ./$$test; done
	@echo "PASS: all ArchV5 host tests"

test_portable: portable_core_compile_test
	./portable_core_compile_test

portable_core_compile_test: portable_core_compile_test.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

test_SWC_PIDController: test_SWC_PIDController.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

test_SWC_EKFAHRS: test_SWC_EKFAHRS.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

test_FailsafeLogic: test_FailsafeLogic.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

test_ControllerRegistry: test_ControllerRegistry.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

test_ModeManager: test_ModeManager.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

test_FlightApplication: test_FlightApplication.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(TEST_BINS)
