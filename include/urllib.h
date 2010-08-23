#ifndef _URLLIB_H
#define _URLLIB_H

#include "common.h"

namespace urllib {

enum request_type {
    GET,
    POST
};

class Request {

    std::string m_uri;
    std::string m_host;
    std::string m_path;
    std::string m_data;
    request_type m_type;

    // for now we're just doing http, but still
    std::string m_proto;
    std::map<std::string, std::string> m_headers;

public:

    Request(const std::string &uri, const std::string &data = "");
    ~Request();
    void addheader(const std::string &key, const std::string &value);
    std::map<std::string, std::string> headers() { return m_headers; }

    request_type type() { return m_type; }
    std::string data() { return m_data; }
    std::string host() { return m_host; }
    std::string path() { return m_path; }
    std::string protocol() { return m_proto; }
};

std::string urlencode_str(const std::string &input);
std::string urlencode(std::map<std::string,std::string> &argmap);
std::string urlopen(const Request &req);

};
#endif 
