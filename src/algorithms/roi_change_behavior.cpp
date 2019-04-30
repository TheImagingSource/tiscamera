#include "roi_change_behavior.h"


const char* roi_change_behavior_to_string (ROI_CHANGE_BEHAVIOR behavior)
{
    switch (behavior)
    {
        case ROI_CHANGE_BEHAVIOR_RESET: return "ROI Change Reset";
        case ROI_CHANGE_BEHAVIOR_UNDEFINED: return "ROI Change Undefined";
        default: return nullptr;
    }
}


ROI_CHANGE_BEHAVIOR roi_change_behavior_from_string (const char* str)
{
    if (strcmp(str, "ROI Change Reset") == 0)
    {
        return ROI_CHANGE_BEHAVIOR_RESET;
    }
    else
    {
        return ROI_CHANGE_BEHAVIOR_UNDEFINED;
    }
}
