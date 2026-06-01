#include "Core/SurvivorArenaLog.h"

//C++ 的老规矩，头文件只负责“吹牛（声明）”，源文件必须负责“兑现（定义）”。这行代码在底层真正分配了内存，创建了这个日志频道的实体。
DEFINE_LOG_CATEGORY(LogSurvivorArena);
