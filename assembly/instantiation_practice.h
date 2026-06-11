// instantiation_practice.h
//
// TODO: 按照公司代码的 INSTANTIATION 模式，把下面两个模块连接起来。
//
// 需要的两个模块：
//   1. modules::OilTempWarning   oil_temp_warning
//      构造函数参数：(oil_temp, app_manager)
//
//   2. modules::SpeedMonitor     speed_monitor
//      构造函数参数：(vehicle_speed, speed_limit, app_manager)
//
// input signals (defined in main.cpp, available as extern):
//   signals::TemperatureSignal  oil_temp
//   signals::SpeedSignal        vehicle_speed
//   signals::Signal<float>      speed_limit
//   framework::Manager          app_manager
//
// 步骤：
//   1. 写 include guard:
//        #if defined(INSTANTIATION_DEFINE) || !defined(INSTANTIATION_PRACTICE_H_)
//        #define INSTANTIATION_PRACTICE_H_
//
//   2. 写条件 include 块（只在 INSTANTIATION_DEFINE 时才 include 类头文件）：
//        #if defined(INSTANTIATION_DEFINE)
//        #include "oil_temp_warning.h"
//        #include "speed_monitor.h"
//        #include "signals/signal.h"
//        #include "framework/manager.h"
//        #endif
//
//   3. 写两行 INSTANTIATION 宏调用
//
//   4. 写 #endif
//
// 为什么 #include 要放在 #if defined(INSTANTIATION_DEFINE) 里？
//   → 普通 include 时只需要知道"有这个名字"(extern)，不需要完整类定义
//   → 定义时才需要完整类定义来调用构造函数

#if defined(INSTANTIATION_DEFINE) || !defined(INSTANTIATION_PRACTICE_H)
#define INSTANTIATION_PRACTICE_H

#if defined(INSTANTIATION_DEFINE)
#include "oil_temp_warning.h"
#include "speed_monitor.h"
#include "signal.h"
#include "manager.h"
#include "module_interface.h"

#else

namespace modules {
class OilTempWarning;
class SpeedMonitor;
}

#endif

INSTANTIATION(
    modules::OilTempWarning,
    oil_temp_warning,
    oil_temp,
    app_manager
)

INSTANTIATION(
    modules::SpeedMonitor,
    speed_monitor,
    vehicle_speed,
    speed_limit,
    app_manager
)

#endif
