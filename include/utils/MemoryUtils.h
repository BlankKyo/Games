// include/utils/MemoryUtils.h
#pragma once

#include <string>
#include <memory>
#include <vector>

class MemoryUtils {
public:
    /**
     * @brief Formats a detailed log string for object creation/destruction.
     * @param action "Constructor" or "Destructor"
     * @param instance The name of the class (e.g., "SortingWidget")
     * @param ObjectSize The size of the object in bytes
     */
    static std::string formatLifecycleLog(
        const std::string& action, 
        void* instance,
        size_t ObjectSize = 0
    );
};


