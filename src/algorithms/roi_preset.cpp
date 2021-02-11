
#include "roi_preset.h"


const char* roi_preset_to_string(ROI_PRESET preset)
{
    switch (preset)
    {
        case ROI_PRESET_FULL_SENSOR:
            return "Full Sensor";
        case ROI_PRESET_CUSTOM_RECTANGLE:
            return "Custom Rectangle";
        case ROI_PRESET_CENTER_50:
            return "Center 50%";
        case ROI_PRESET_CENTER_25:
            return "Center 25%";
        case ROI_PRESET_BOTTOM_HALF:
            return "Bottom Half";
        case ROI_PRESET_TOP_HALF:
            return "Top Half";
        default:
            return nullptr;
    }
}

ROI_PRESET roi_preset_from_string(const char* str)
{
    if (strcmp(str, "Custom Rectangle") == 0)
    {
        return ROI_PRESET_CUSTOM_RECTANGLE;
    }
    else if (strcmp(str, "Center 25%") == 0)
    {
        return ROI_PRESET_CENTER_25;
    }
    else if (strcmp(str, "Center 50%") == 0)
    {
        return ROI_PRESET_CENTER_50;
    }
    else if (strcmp(str, "Bottom Half") == 0)
    {
        return ROI_PRESET_BOTTOM_HALF;
    }
    else if (strcmp(str, "Top Half") == 0)
    {
        return ROI_PRESET_TOP_HALF;
    }
    else
    {
        // if (strcmp(str, "Full Sensor") == 0)
        return ROI_PRESET_FULL_SENSOR;
    }
}
