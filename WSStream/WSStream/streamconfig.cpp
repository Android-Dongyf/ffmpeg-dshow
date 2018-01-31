#include "streamconfig.h"
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QTextCodec>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;


string StreamConfig::video_input_fmt;
int StreamConfig::video_index;
string StreamConfig::video_desc;
string StreamConfig::video_framerate;
int StreamConfig::video_width;
int StreamConfig::video_height;
int StreamConfig::video_crop_width;
int StreamConfig::video_crop_height;
string StreamConfig::video_fmt;
string StreamConfig::filter_description;
int StreamConfig::video_encode_rate;
int StreamConfig::video_encode_width;
int StreamConfig::video_encode_height;
string StreamConfig::audio_input_fmt;
string StreamConfig::audio_desc;
int StreamConfig::audio_input_channel;
int StreamConfig::audio_sample_rate;
string StreamConfig::stream_output_fmt;
int StreamConfig::stream_output_framerate;
string StreamConfig::stream_output_addr_master;
string StreamConfig::stream_output_addr_slave;
int StreamConfig::stream_output_port;
string StreamConfig::stream_output_secret;
string StreamConfig::stream_output_index;
string StreamConfig::jpeg_encoder_fmt;
string StreamConfig::jpeg_file_name;
string StreamConfig::machine_code;
int StreamConfig::http_svr_port;
string StreamConfig::http_svr_addr;


StreamConfig::StreamConfig()
{
    loadConfigFromFile();
}

void StreamConfig::loadConfigFromFile(const string fileName){
    qDebug() << "file_name: " << fileName.c_str();

    QFile file(fileName.c_str());
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "read file " << fileName.c_str() << " error: " << file.errorString();
        return;
    }

    QTextStream in(&file);
    QString str = in.readAll();
    QByteArray data = str.toUtf8();
    char *val = data.data();
    //qDebug() << "val: " <<  val;

    bool ret = parseByJson(val);
    if(!ret)
        qDebug() << "parseByJson fail.";

    file.close();

    //dump();
}

bool StreamConfig::parseByJson(const char *val){
    Document document;

    if (document.Parse<0>(val).HasParseError())
    {
        qDebug() << "parseByJson parse error. ";
        return false;
    }

    if(document.HasMember("video_input_fmt")) {
        const char *str = document["video_input_fmt"].GetString();
        string val(str);
        video_input_fmt = val;
    }

    if(document.HasMember("video_index")) {
        int val = document["video_index"].GetInt();
        video_index = val;
    }

    if(document.HasMember("video_desc")) {
        const char *str = document["video_desc"].GetString();
        string val(str);
        video_desc = val;
    }

    if(document.HasMember("video_framerate")) {
        const char *str = document["video_framerate"].GetString();
        string val(str);
        video_framerate = val;
    }

    if(document.HasMember("video_width")) {
        int val = document["video_width"].GetInt();
        video_width = val;
    }

    if(document.HasMember("video_height")) {
        int val = document["video_height"].GetInt();
        video_height = val;
    }

    if(document.HasMember("filter_description")) {
        const char *str = document["filter_description"].GetString();
        string val(str);
        filter_description = val;
    }

    if(document.HasMember("video_crop_width")) {
        int val = document["video_crop_width"].GetInt();
        video_crop_width = val;
    }

    if(document.HasMember("video_crop_height")) {
        int val = document["video_crop_height"].GetInt();
        video_crop_height = val;
    }

    if(document.HasMember("video_encode_width")) {
        int val = document["video_encode_width"].GetInt();
        video_encode_width = val;
    }

    if(document.HasMember("video_encode_height")) {
        int val = document["video_encode_height"].GetInt();
        video_encode_height = val;
    }

    if(document.HasMember("video_fmt")) {
        const char *str = document["video_fmt"].GetString();
        string val(str);
        video_fmt = val;
    }

    if(document.HasMember("video_encode_rate")) {
        int val = document["video_encode_rate"].GetInt();
        video_encode_rate = val;
    }

    if(document.HasMember("audio_input_fmt")) {
        const char *str = document["audio_input_fmt"].GetString();
        string val(str);
        audio_input_fmt = val;
    }

    if(document.HasMember("audio_desc")) {
        const char *str = document["audio_desc"].GetString();
        string val(str);
        audio_desc = val;
    }

    if(document.HasMember("audio_input_channel")) {
        int val = document["audio_input_channel"].GetInt();
        audio_input_channel = val;
    }

    if(document.HasMember("audio_sample_rate")) {
        int val = document["audio_sample_rate"].GetInt();
        audio_sample_rate = val;
    }

    if(document.HasMember("stream_output_fmt")) {
        const char *str = document["stream_output_fmt"].GetString();
        string val(str);
        stream_output_fmt = val;
    }

    if(document.HasMember("stream_output_framerate")) {
        int val = document["stream_output_framerate"].GetInt();
        stream_output_framerate = val;
    }

    if(document.HasMember("stream_output_addr_master")) {
        const char *str = document["stream_output_addr_master"].GetString();
        string val(str);
        stream_output_addr_master = val;
    }

    if(document.HasMember("stream_output_addr_slave")) {
        const char *str = document["stream_output_addr_slave"].GetString();
        string val(str);
        stream_output_addr_slave = val;
    }

    if(document.HasMember("stream_output_port")) {
        int val = document["stream_output_port"].GetInt();
        stream_output_port = val;
    }

    if(document.HasMember("stream_output_secret")) {
        const char *str = document["stream_output_secret"].GetString();
        string val(str);
        stream_output_secret = val;
    }

    if(document.HasMember("stream_output_index")) {
        const char *str = document["stream_output_index"].GetString();
        string val(str);
        stream_output_index = val;
    }

    if(document.HasMember("jpeg_encoder_fmt")) {
        const char *str = document["jpeg_encoder_fmt"].GetString();
        string val(str);
        jpeg_encoder_fmt = val;
    }

    if(document.HasMember("jpeg_file_name")) {
        const char *str = document["jpeg_file_name"].GetString();
        string val(str);
        jpeg_file_name = val;
    }

    if(document.HasMember("machine_code")) {
        const char *str = document["machine_code"].GetString();
        string val(str);
        machine_code = val;
    }

    if(document.HasMember("http_svr_port")) {
        int val = document["http_svr_port"].GetInt();
        http_svr_port = val;
    }

    if(document.HasMember("http_svr_addr")) {
        const char *str = document["http_svr_addr"].GetString();
        string val(str);
        http_svr_addr = val;
    }

    return true;
}

string StreamConfig::video_input_fmt_val(){
    return video_input_fmt;
}

int StreamConfig::video_index_val(){
    return video_index;
}

string StreamConfig::video_desc_val(){
    return video_desc;
}

string StreamConfig::video_framerate_val(){
    return video_framerate;
}

int StreamConfig::video_width_val(){
    return video_width;
}

int StreamConfig::video_height_val(){
    return video_height;
}

int StreamConfig::video_crop_width_val(){
    return video_crop_width;
}

int StreamConfig::video_crop_height_val(){
    return video_crop_height;
}

string StreamConfig::filter_description_val(){
    return filter_description;
}

int StreamConfig::video_encode_width_val(){
    return video_encode_width;
}

int StreamConfig::video_encode_height_val(){
    return video_encode_height;
}

string StreamConfig::video_size_str_val(){
    char buf[128] = {0};

    sprintf(buf, "%dx%d", video_width, video_height);
    string size(buf);
    return size;
}

string StreamConfig::video_fmt_val(){
    return video_fmt;
}

int StreamConfig::video_encode_rate_val(){
    return video_encode_rate;
}

string StreamConfig::audio_input_fmt_val(){
    return audio_input_fmt;
}

string StreamConfig::audio_desc_val(){
    return audio_desc;
}

int StreamConfig::audio_input_channel_val(){
    return audio_input_channel;
}

int StreamConfig::audio_sample_rate_val(){
    return audio_sample_rate;
}

string StreamConfig::stream_output_fmt_val(){
    return stream_output_fmt;
}

int StreamConfig::stream_output_framerate_val(){
    return stream_output_framerate;
}

string StreamConfig::stream_output_addr_master_val(){
    return stream_output_addr_master;
}

string StreamConfig::stream_output_addr_slave_val(){
    return stream_output_addr_slave;
}

string StreamConfig::http_svr_addr_val(){
    return http_svr_addr;
}

int StreamConfig::stream_output_port_val(){
    return stream_output_port;
}

string StreamConfig::stream_output_secret_val(){
    return stream_output_secret;
}

string StreamConfig::stream_output_index_val(){
    return stream_output_index;
}

string StreamConfig::jpeg_encoder_fmt_val(){
    return jpeg_encoder_fmt;
}

string StreamConfig::jpeg_file_name_val(){
    return jpeg_file_name;
}

string StreamConfig::machine_code_val(){
    return machine_code;
}

int StreamConfig::http_svr_port_val(){
    return http_svr_port;
}

void StreamConfig::dump(){
    qDebug() << "video_input_fmt: " << video_input_fmt.c_str();
    qDebug() << "video_desc: " << video_desc.c_str();
    qDebug() << "video_framerate: " << video_framerate.c_str();
    qDebug() << "video_width: " << video_width;
    qDebug() << "video_height: " << video_height;
    qDebug() << "video_size: " << video_size_str_val().c_str();
    qDebug() << "video_fmt: " << video_fmt.c_str();
    qDebug() << "audio_input_fmt: " << audio_input_fmt.c_str();
    qDebug() << "audio_desc: " << audio_desc.c_str();
    qDebug() << "audio_input_channel: " << audio_input_channel;
    qDebug() << "audio_sample_rate: " << audio_sample_rate;
    qDebug() << "stream_output_fmt: " << stream_output_fmt.c_str();
    qDebug() << "stream_output_framerate: " << stream_output_framerate;
    qDebug() << "stream_output_addr_master: " << stream_output_addr_master.c_str();
    qDebug() << "stream_output_addr_slave: " << stream_output_addr_slave.c_str();
    qDebug() << "stream_output_port: " << stream_output_port;
    qDebug() << "stream_output_secret: " << stream_output_secret.c_str();
    qDebug() << "stream_output_index: " << stream_output_index.c_str();
    qDebug() << "jpeg_encoder_fmt: " << jpeg_encoder_fmt.c_str();
    qDebug() << "jpeg_file_name: " << jpeg_file_name.c_str();
    qDebug() << "machine_code: " << machine_code.c_str();
    qDebug() << "http_svr_port: " << http_svr_port;
}
