#include "common.h"
#include "urllib.h"

namespace urllib {

/**
Split a protocol://hostname/path into a vector of (protocol, restofurl)
modeled after urllib's implementation.
*/

vector<std::string> splittype(const std::string &s)
{
    // protocol
    std::vector<std::string> values;
    boost::regex re("^([^/:]+):(.+?)");
    boost::cmatch matches;

    if (boost::regex_match(s.c_str(), matches, re) && matches.size() == 3)
    {
        // matches[0] contains the original string.  matches[n]
        // contains a sub_match object for each matching
        // subexpression
        for (int i = 1; i < matches.size(); i++)
        {
            // sub_match::first and sub_match::second are iterators that
            // refer to the first and one past the last chars of the
            // matching subexpression
            string match(matches[i].first, matches[i].second);
            values.push_back(match);
        }
    }

    return values;
}

vector<std::string> splithost(const std::string &s)
{
    // protocol
    std::vector<std::string> values;
    boost::regex re("^//([^/?]*)(.*)$");
    boost::cmatch matches;

    if (boost::regex_match(s.c_str(), matches, re) && matches.size() == 3)
    {
        // matches[0] contains the original string.  matches[n]
        // contains a sub_match object for each matching
        // subexpression
        for (int i = 1; i < matches.size(); i++)
        {
            // sub_match::first and sub_match::second are iterators that
            // refer to the first and one past the last chars of the
            // matching subexpression
            string match(matches[i].first, matches[i].second);
            cout << "\tmatches[" << i << "] = " << match << endl;
            values.push_back(match);
        }
    }

    return values;
}



Request::Request(const std::string &uri, const std::string &data) : m_uri(uri), m_data(data)
{
    
}

// really retardedly basic version of python's urlopen
void urlopen(const std::string &uri, const std::string &data = "")
{
    using boost::asio::ip::tcp;

    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(uri, "http");

    tcp::socket socket(io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;

    while (error && endpoint_iterator != end)
    {
        socket.close();
        socket.connect(*endpoint_iterator++, error);
    }

    if (error)
      throw boost::system::system_error(error);

    boost::asio::streambuf request;
    std::ostream request_stream(&request);

}



std::string urlencode_str(const std::string &input)
{
    char buffer[4];
    std::string encoded;

    // XXX: Would a stringstream be faster?
    for(std::string::const_iterator iter = input.begin(); iter != input.end(); iter++)
    {
        unsigned short ascii = *iter;

        if((ascii > 47 && ascii < 58) || (ascii > 64 && ascii < 91) || (ascii > 96 && ascii < 123))
            encoded += *iter;

        else if (ascii == 32)
            encoded += "+";

        else
        {
            ::snprintf(buffer, 4, "%%%.2x", ascii);
            encoded += buffer;
        }
    }

    return encoded;
}

/*
    Convert a std::map of two std::string objects to a url-encoded string.
*/

std::string urlencode(std::map<std::string,std::string> &argmap)
{
    std::string encoded;

    // XXX: Would a stringstream be faster?
    for (std::map<std::string, std::string>::iterator iter = argmap.begin(); iter != argmap.end(); iter++)
        encoded += urlencode_str(iter->first) + "=" + urlencode_str(iter->second) + "&";

    // Remove the last ampersand.
    return encoded.erase(encoded.size() -1, 1);
}

};
