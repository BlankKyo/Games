// src/utils/MemoryUtils.cpp
#include "utils/MemoryUtils.h"
#include <QString>
#include "utils/Logger.h"



std::string MemoryUtils::formatLifecycleLog(
    const std::string& action, 
    void* instance,
    size_t ObjectSize
) {
    uintptr_t address = reinterpret_cast<uintptr_t>(instance);

    return action + " at " + QString("0x%1").arg(address, 0, 16).toStdString() + 
           " | Size ~" + std::to_string(ObjectSize) + " bytes ";
}
