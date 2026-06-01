#pragma once

#include "CoreMinimal.h"

/*
*作用： 这是一句向全剧组（所有引入这个头文件的其他类）广播的声明。告诉大家：“嘿！我们项目里有一个专门用来写日志的频道，叫 LogSurvivorArena ！”

参数解析：
LogSurvivorArena：你们自定义的频道名字。通常以 Log 开头加上项目名。
Log：默认的日志级别（Verbosity Level）。在编辑器里跑游戏时，默认显示这个级别（比如纯粹的信息记录）。
All：编译进包的最高级别。All 意味着无论你是打印 Error（红字）、Warning（黄字）还是 Log（白字），在打包游戏时都会被编译进去。
 */
DECLARE_LOG_CATEGORY_EXTERN(LogSurvivorArena, Log, All);
