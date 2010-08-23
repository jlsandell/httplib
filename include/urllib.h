#ifndef _URLLIB_H
#define _URLLIB_H

#include "common.h"

namespace urllib {

class Request {
    std::string m_uri;
    std::string m_data;

    public:
        Request(std::string &uri, std::string &data = "");
};

std::string urlencode_str(const std::string &input);
std::string urlencode(std::map<std::string,std::string> &argmap);

void urlopen(const Request &request, const std::string &data);

};
#endif 
