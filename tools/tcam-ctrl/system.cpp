
#include "system.h"

#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace
{

std::string exec(const std::string& cmd)
{

    std::array<char, 128> buffer;
    std::string result;

    // std::cout << "Opening reading pipe" << std::endl;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Couldn't start command." << std::endl;
        return "";
    }
    while (fgets(buffer.data(), 128, pipe) != NULL)
    {
        // std::cout << "Reading..." << std::endl;
        result += buffer.data();
    }
    /*auto returnCode =*/pclose(pipe);

    // std::cout << result << std::endl;
    // std::cout << returnCode << std::endl;

    return result;
}


void check_package(const std::string& pckg_name)
{
    std::string cmd = "dpkg --status " + pckg_name + " 2>&1";

    std::string dpkg = exec(cmd);

    if (dpkg.find("is not installed") != std::string::npos)
    {
        std::cout << pckg_name << " is not installed."
                  << "\n\n";
    }
    else
    {
        std::cout << dpkg << "\n";
    }
}

void check_system_info(const std::string& info)
{
    if (info.find("is not installed") != std::string::npos)
    {
        std::cout << "Executing '" << info << "' requires sudo."
                  << "\n\n";
    }

    std::string cmd = info + " 2>&1";

    std::string out = exec(cmd);

    std::cout << "======= " << info << "\n\n";
    std::cout << out << "\n\n";
}


} // namespace

namespace tcam::tools
{


void print_packages()
{
    check_package("tiscamera");
    check_package("tiscamera-tcamprop");
    check_package("tiscamera-dutils");
    check_package("tispimipisrc");
    check_package("tistegrasrc");
    check_package("ic_barcode");
}


void print_system_info_general()
{
    check_system_info("lsb_release -a");
    check_system_info("uname -a");
    // check_system_info("sudo lshw");
    check_system_info("lscpu");
}


void print_system_info_gige()
{
    check_system_info("ip a");
}


void print_system_info_usb()
{

    check_system_info("lsusb");
    check_system_info("lsusb -t");
}


void print_system_info()
{
    print_system_info_general();
    print_system_info_usb();
    print_system_info_gige();

    std::cout << "Please review the printed information to ensure" << std::endl
              << "that nothing you consider confidential is given to other parties." << std::endl;
}

} // namespace tcam::tools
