TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -pthread
QMAKE_CXXFLAGS += -std=c++1z -pthread -Wall -pedantic
QMAKE_CXXFLAGS_CXX11 = -std=c++1z    # Prevent QMake from overriding c++1z.
QMAKE_CXXFLAGS_GNUCXX11 = -std=c++1z #
QMAKE_CXXFLAGS_CXX14 = -std=c++1z    #
QMAKE_CXXFLAGS_GNUCXX14 = -std=c++1z #

SOURCES += \
    controllers/ewalk/ewalkcontroller.cpp \
    controllers/ewalk/ewalkhipphasebasedcontroller.cpp \
    controllers/ewalk/ewalkreactivecontroller.cpp \
    controllers/ewalk/ewalktimebasedtorqueprofile.cpp \
    drivers/canbus.cpp \
    drivers/gyems.cpp \
    main.cpp \
    drivers/uart.cpp \
    server.cpp \
    communication.cpp \
    drivers/motorboard.cpp \
    drivers/i2c.cpp \
    drivers/hmc5883l.cpp \
    drivers/mpu60x0.cpp \
    drivers/spi.cpp \
    drivers/ads1148.cpp \
    drivers/ads7844.cpp \
    drivers/adc124s01.cpp \
    drivers/adc.cpp \
    drivers/ledstatusindicator.cpp \
    drivers/crutchboard.cpp \
    lib/batterymonitor.cpp \
    drivers/gpio.cpp \
    lib/bytebuffer.cpp \
    lib/utils.cpp \
    drivers/pwm.cpp \
    drivers/polarhrs.cpp \
    drivers/pcf8523.cpp \
    drivers/amt20.cpp \
    drivers/as5048b.cpp \
    lib/peripheral.cpp \
    lib/quadloadcells.cpp \
    lib/stateestimator.cpp \
    lib/debugstream.cpp \
    lib/scheduler.cpp \
    lib/trajectorygenerator.cpp \
    lib/imu.cpp \
    controllers/controller.cpp \
    controllers/hibso/hibso.cpp \
    controllers/hibso/hibsoafocontroller.cpp \
    controllers/hibso/hibsoadaptiveoscillator.cpp \
    controllers/twiice/twiicecontroller.cpp \
    controllers/twiice/twiicemode.cpp \
    controllers/twiice/sittingmode.cpp \
    controllers/twiice/adjustsittingmode.cpp \
    controllers/twiice/periodicgaitmode.cpp \
    controllers/twiice/stonesmode.cpp \
    controllers/twiice/freemode.cpp \
    controllers/twiice/fixedmode.cpp \
    controllers/twiice/stepovermode.cpp \
    controllers/twiice/platformmode.cpp \
    controllers/twiice/roughterrainmode.cpp \
    controllers/twiice/roughterrainmode2.cpp \
    controllers/twiice/touchdesignerserver.cpp \
    controllers/twiice/periodicgaitmode2.cpp \
    controllers/twiice/standingbalancemode.cpp \
    controllers/twiice/button.cpp \
    controllers/twiice/balanceperturbator.cpp \
    controllers/twiice/kneelingmode.cpp \
    controllers/captur/capturcontroller.cpp \
    controllers/autonomyo/autonomyocontroller.cpp \
    controllers/autonomyo/autonomyojoint.cpp \
    controllers/imu_calibrator/imucalibrator.cpp \
    controllers/test/testcontroller.cpp \
    controllers/twiice-h1/twiiceh1controller.cpp \
    controllers/twiice-h1/runnerangularsensorsmanager.cpp \
    lib/beeper.cpp \
    lib/configfile.cpp \
    lib/fourierseries.cpp \
    lib/syncvar/syncvar.cpp \
    lib/syncvar/syncvarmanager.cpp \
    lib/filters/basicfilter.cpp \
    lib/filters/lowpassfilter.cpp \
    lib/filters/runningmean.cpp \
    lib/filters/derivator.cpp \
    lib/filters/slewratefilter.cpp \
    lib/filters/hysteresisfilter.cpp \
    lib/pid.cpp \
    lib/vibrator.cpp \
    lib/binarysignalfilter.cpp \
    lib/bezierspline.cpp

HEADERS += \
    controllers/ewalk/ewalkcontroller.h \
    controllers/ewalk/ewalkdefinitions.h \
    controllers/ewalk/ewalkhipphasebasedcontroller.h \
    controllers/ewalk/ewalkreactivecontroller.h \
    controllers/ewalk/ewalktimebasedtorqueprofile.h \
    drivers/canbus.h \
    drivers/gyems.h \
    drivers/uart.h \
    server.h \
    public_definitions.h \
    communication.h \
    drivers/motorboard.h \
    drivers/i2c.h \
    drivers/hmc5883l.h \
    lib/vec3.h \
    drivers/mpu60x0.h \
    drivers/spi.h \
    drivers/ads1148.h \
    drivers/ads7844.h \
    drivers/adc124s01.h \
    drivers/adc.h \
    drivers/ledstatusindicator.h \
    lib/batterymonitor.h \
    drivers/gpio.h \
    lib/bytebuffer.h \
    lib/utils.h \
    lib/syncvar.h \
    drivers/pwm.h \
    drivers/polarhrs.h \
    drivers/pcf8523.h \
    drivers/amt20.h \
    drivers/crutchboard.h \
    drivers/as5048b.h \
    lib/peripheral.h \
    lib/quadloadcells.h \
    lib/vecn.h \
    lib/vec4.h \
    lib/vec3.h \
    lib/vec2.h \
    lib/stateestimator.h \
    lib/lowpassfilter.h \
    lib/debugstream.h \
    lib/scheduler.h \
    lib/trajectorygenerator.h \
    lib/imu.h \
    controllers/controller.h \
    controllers/hibso/hibso.h \
    controllers/hibso/hibsoafocontroller.h \
    controllers/hibso/hibsoadaptiveoscillator.h \
    controllers/twiice/fixedmode.h \
    controllers/twiice/twiicecontroller.h \
    controllers/twiice/twiicedefinitions.h \
    controllers/twiice/twiicemode.h \
    controllers/twiice/twiiceutils.h \
    controllers/twiice/sittingmode.h \
    controllers/twiice/adjustsittingmode.h \
    controllers/twiice/periodicgaitmode.h \
    controllers/twiice/stonesmode.h \
    controllers/twiice/freemode.h \
    controllers/twiice/stepovermode.h \
    controllers/twiice/platformmode.h \
    controllers/twiice/roughterrainmode.h \
    controllers/twiice/roughterrainmode2.h \
    controllers/twiice/periodicgaitmode2.h \
    controllers/twiice/standingbalancemode.h \
    controllers/twiice/kneelingmode.h \
    controllers/twiice/twiiceconfig.h \
    controllers/twiice/trajectories/trajectories.h \
    controllers/twiice/trajectories/traj_fast_gait.h \
    controllers/twiice/trajectories/traj_normal_gait.h \
    controllers/twiice/trajectories/traj_slope_down.h \
    controllers/twiice/trajectories/traj_slope_up.h \
    controllers/twiice/trajectories/traj_slow_gait.h \
    controllers/twiice/trajectories/traj_stride_over_gait.h \
    controllers/twiice/trajectories/traj_rough_terrain.h \
    controllers/twiice/trajectories/traj_platform.h \
    controllers/twiice/trajectories/traj_stairs.h \
    controllers/twiice/trajectories/traj_stones.h \
    controllers/twiice/trajectories/traj_fast_stairs.h \
    controllers/twiice/trajectories/traj_fast_stairs_descent.h \
    controllers/twiice/trajectories/traj_sofa_standup.h \
    controllers/twiice/trajectories/traj_stool_standup.h \
    controllers/twiice/touchdesignerserver.h \
    controllers/twiice/button.h \
    controllers/twiice/balanceperturbator.h \
    controllers/captur/capturcontroller.h \
    controllers/autonomyo/autonomyocontroller.h \
    controllers/autonomyo/autonomyojoint.h \
    controllers/autonomyo/trajectories/traj_test.h \
    controllers/imu_calibrator/imucalibrator.h \
    controllers/test/testcontroller.h \
    controllers/twiice-h1/twiiceh1controller.h \
    controllers/twiice-h1/runnerangularsensorsmanager.h \
    controllers/twiice-h1/runnerconstants.h \
    config/config.h \
    lib/syncvar/syncvar.h \
    lib/syncvar/syncvarfunc.h \
    lib/syncvar/syncvarmanager.h \
    lib/syncvar/syncvaraddr.h \
    lib/runningmean.h \
    lib/basicfilter.h \
    lib/beeper.h \
    lib/configfile.h \
    lib/filters/basicfilter.h \
    lib/filters/lowpassfilter.h \
    lib/filters/runningmean.h \
    lib/filters/derivator.h \
    lib/filters/slewratefilter.h \
    lib/filters/hysteresisfilter.h \
    lib/pid.h \
    lib/fourierseries.h \
    lib/vibrator.h \
    lib/binarysignalfilter.h \
    lib/bezierspline.h

# Uncomment to profile with gprof.
#QMAKE_CFLAGS+=-pg
#QMAKE_CXXFLAGS+=-pg
#QMAKE_LFLAGS+=-pg

# Settings specific for the BeagleBone Black (optimizations and deployment).
linux-arm-gnueabihf-g++ {
    QMAKE_CXXFLAGS += -march=armv7-a -mtune=cortex-a8 -mfloat-abi=hard -mfpu=neon -ffast-math
    #QMAKE_LFLAGS += -static-libstdc++ # Use this if your cross-compiler is more recent than the BeagleBone libstdc++. This significantly increases the size of the executable.

    Debug: QMAKE_CXXFLAGS += -Og
    Release: QMAKE_CXXFLAGS += -O3 -funroll-loops

    target.path = /root/WalkiSoftware/BeagleBone
    INSTALLS += target
}
