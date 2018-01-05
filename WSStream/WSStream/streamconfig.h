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
    static string video_desc_val();
    static string video_framerate_val();
    static int video_width_val();
    static int video_height_val();
    static string video_size_str_val();
    static string video_fmt_val();
    static string audio_input_fmt_val();
    static string audio_desc_val();
    static int audio_input_channel_val();
    static int audio_sample_rate_val();
    static string stream_output_fmt_val();
    static int stream_output_framerate_val();
    static string stream_output_addr_val();
    static int stream_output_port_val();
    static string stream_output_secret_val();
    static string stream_output_index_val();
    static string jpeg_encoder_fmt_val();
    static string jpeg_file_name_val();
    static string machine_code_val();
    static int http_svr_port_val();
    void dump();
private:
    void loadConfigFromFile(const string fileName = "./stream.conf");
    bool parseByJson(const char *val);

private:
    static string video_input_fmt;
    static string video_desc;
    static string video_framerate;
    static int video_width;
    static int video_height;
    static string video_fmt;
    static string audio_input_fmt;
    static string audio_desc;
    static int audio_input_channel;
    static int audio_sample_rate;
    static string stream_output_fmt;
    static int stream_output_framerate;
    static string stream_output_addr;
    static int stream_output_port;
    static string stream_output_secret;
    static string stream_output_index;
    static string jpeg_encoder_fmt;
    static string jpeg_file_name;
    static string machine_code;
    static int http_svr_port;
};

#endif // STREAMCONFIG_H
