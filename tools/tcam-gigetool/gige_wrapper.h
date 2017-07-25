#ifndef __GIGE_WRAPPER_H__
#define __GIGE_WRAPPER_H__

typedef void(*discover_callback_t)(struct tcam_camera);
typedef void(*upload_callback_t)(const char *msg, int progress);


#define SUCCESS           0x0
#define FAILURE           0x8000
#define NO_DEVICE         0x8001
#define INVALID_PARAMETER 0x8002

extern "C"
{
    struct tcam_camera
    {
        char model_name[64];
        char serial_number[64];
        char current_ip[16];
        char current_gateway[16];
        char current_netmask[16];
        char persistent_ip[16];
        char persistent_gateway[16];
        char persistent_netmask[16];
        char user_defined_name[64];
        char firmware_version[64];
        char mac_address[64];
        char interface_name[64];
        int is_static_ip;
        int is_dhcp_enabled;
        int is_reachable;
        int is_busy;
    };

    void init(void);
    int get_camera_list(discover_callback_t callback, int get_persistent_values);
    int get_camera_details(char *identifier, struct tcam_camera *tcam);
    int set_persistent_parameter_s(char *identifier, char *key, char *value);
    int set_persistent_parameter_i(char *identifier, char *key, int value);
    int rescue(char *mac, char* ip, char *netmask, char *gateway);
    int upload_firmware(char *identifier, char *path, upload_callback_t callback);
}



#endif// __GIGE_WRAPPER_H__