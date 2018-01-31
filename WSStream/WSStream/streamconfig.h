#ifndef STREAMCONFIG_CUST_H
#define STREAMCONFIG_CUST_H
#include <string>
using namespace std;

#define CONFIG_FILE_NAME "./stream.conf"

class StreamConfig
{
public:
    StreamConfig();

    static string video_input_fmt_val();
    static int video_index_val();
    static string video_desc_val();
    static string video_framerate_val();
    static int video_width_val();
    static int video_height_val();
    static int video_crop_width_val();
    static int video_crop_height_val();
    static int video_encode_width_val();
    static int video_encode_height_val();
    static string video_size_str_val();
    static string filter_description_val();
    static string video_fmt_val();
    static int video_encode_rate_val();
    static string audio_input_fmt_val();
    static string audio_desc_val();
    static int audio_input_channel_val();
    static int audio_sample_rate_val();
    static string stream_output_fmt_val();
    static int stream_output_framerate_val();
    static string stream_output_addr_master_val();
    static string stream_output_addr_slave_val();
    static int stream_output_port_val();
    static string stream_output_secret_val();
    static string stream_output_index_val();
    static string jpeg_encoder_fmt_val();
    static string jpeg_file_name_val();
    static string machine_code_val();
    static int http_svr_port_val();
    static string http_svr_addr_val();
    void dump();
private:
    void loadConfigFromFile(const string fileName = "./stream.conf");
    bool parseByJson(const char *val);

private:
    static string video_input_fmt;
    static int video_index;
    static string video_desc;
    static string video_framerate;
    static int video_width;
    static int video_height;
    static int video_crop_width;
    static int video_crop_height;
    static string filter_description;
    static int video_encode_width;
    static int video_encode_height;
    static string video_fmt;
    static int video_encode_rate;
    static string audio_input_fmt;
    static string audio_desc;
    static int audio_input_channel;
    static int audio_sample_rate;
    static string stream_output_fmt;
    static int stream_output_framerate;
    static string stream_output_addr_master;
    static string stream_output_addr_slave;
    static int stream_output_port;
    static string stream_output_secret;
    static string stream_output_index;
    static string jpeg_encoder_fmt;
    static string jpeg_file_name;
    static string machine_code;
    static int http_svr_port;
    static string http_svr_addr;
};

#endif // STREAMCONFIG_H
