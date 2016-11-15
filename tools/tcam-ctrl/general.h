

#include <string>



inline std::string extract_directory (const std::string& path)
{
    return path.substr( 0, path.find_last_of( '/' ) +1 );
}

inline std::string extract_filename (const std::string& path)
{
    return path.substr( path.find_last_of( '/' ) +1 );
}

inline std::string change_extension (const std::string& path, const std::string& ext)
{
    std::string filename = extract_filename( path );
    return extract_directory( path ) +filename.substr( 0, filename.find_last_of( '.' ) ) +ext;
}

bool save_device_list (const std::string& filename);

bool save_device_settings (const std::string& serial, const std::string& filename);

bool load_device_settings (const std::string& serial, const std::string& filename);
