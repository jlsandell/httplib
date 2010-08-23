#include "common.h"
#include "urllib.h"

namespace urllib {

/**
Split a protocol://hostname/path into a vector of (protocol, restofurl)
modeled after urllib's implementation.
*/

std::vector<std::string> splittype(const std::string &s)
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
            std::string match(matches[i].first, matches[i].second);
            values.push_back(match);
        }
    }

    return values;
}

std::vector<std::string> splithost(const std::string &s)
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
            std::string match(matches[i].first, matches[i].second);
            values.push_back(match);
        }
    }

    return values;
}

Request::Request(const std::string &uri, const std::string &data) : m_uri(uri), m_data(data)
{
    m_headers["User-Agent"] = "C++ urllib/v0.0.1";

    // get first set of two-tuples from uri
    std::vector<std::string> protohost = splittype(uri);

    if (protohost.size() != 2)
        throw std::runtime_error("Invalid url (proto://host)");
    
    m_proto = protohost[0];
    // get second two-tuple
    std::vector<std::string> hosturi = splithost(protohost[1]);

    if (hosturi.size() != 2)
        throw std::runtime_error("Invalid url (host/path)");

    m_host = hosturi[0];
    m_path = hosturi[1];

}

Request::~Request() {}

void Request::addheader(const std::string &key, const std::string &value)
{
    m_headers[key] = value;
}

// really retardedly basic version of python's urlopen
std::string urlopen(Request &req)
{
    using boost::asio::ip::tcp;
    using boost::format;

    std::ostringstream response_string;

    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(req.host(), "http");

    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

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

    // no data string given, so it's a get request.
    //if (data == "")
    //XXX: default to get until we implement POST

    request_stream << format("GET %s HTTP/1.0\r\n") % req.path();
    request_stream << format("Host: %s\r\n") % req.host();

    std::map<std::string, std::string> headers = req.headers();

    for (std::map<std::string, std::string>::iterator iter = headers.begin(); iter != headers.end(); iter++)
        request_stream << format("%s: %s\r\n") % iter->first % iter->second;

    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    boost::asio::write(socket, request);

    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");

    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        throw std::runtime_error("Invalid response!\n");

    if (status_code != 200)
    {
        response_string << "Response returned with status code " << status_code;
        return response_string.str();
    }

    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;

    while (std::getline(response_stream, header) && header != "\r")
    {
        //std::cout << "OOK\t::\t" << header << "\n";
    }
    //std::cout << "\n";

    // Write whatever content we already have to output.
    if (response.size() > 0)
    {
        response_string << response;
        //std::cout << &response;
    }

    // Read until EOF, writing data to output as we go.
    while (boost::asio::read(socket, response,
          boost::asio::transfer_at_least(1), error))
    {

        //std::cout << &response;
        response_string << response;
    }
    if (error != boost::asio::error::eof)
      throw boost::system::system_error(error);

    return response_string.str();
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

// to test functionality.
#if 1
int main(int argc, char **argv)
{
    using namespace std;
    using boost::format;
    urllib::Request req("http://dev.local.lan/");
    std::cout << req.host() << std::endl;
    //std::cout << urllib::urlopen(req) << std::endl;
    return 0;
}
#endif 
