#ifndef _URLLIB_H
#define _URLLIB_H

#include "common.h"

namespace httplib {

enum request_type {
    GET,
    POST
};

class Request {

    std::string m_uri;
    std::string m_host;
    std::string m_path;

    // http post
    std::string m_payload;
    std::map<std::string, std::string> m_data;

    // for now we're just doing http, but still
    std::string m_proto;
    request_type m_type;

    std::map<std::string, std::string> m_headers;

    Request(const Request &);
    Request & operator=(const Request &);

public:

    Request(const std::string &uri);
    Request(const std::string &uri, const std::map<std::string, std::string> &postdata);
    ~Request();

    void init(const std::string &uri);
    void addheader(const std::string &key, const std::string &value);
    std::string getheader(const std::string &key);
    std::map<std::string, std::string> headers() { return m_headers; }

    request_type type() { return m_type; }

    std::string host() { return m_host; }
    std::string path() { return m_path; }
    std::string protocol() { return m_proto; }
    std::map<std::string, std::string> data() { return m_data; }
    std::string payload() { return m_payload; }
};

std::string urlencode_str(const std::string &input);
std::string urlencode(const std::map<std::string,std::string> &argmap);
std::string urlopen(const Request &req);
std::string b64encode(std::string &input);

};
#endif 
