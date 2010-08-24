#include "common.h"
#include "httplib.h"

namespace httplib {

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
    // hostname, path
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

std::string urlencode(const std::map<std::string,std::string> &argmap)
{
    std::string encoded;

    // XXX: Would a stringstream be faster?
    for (std::map<std::string, std::string>::const_iterator iter = argmap.begin(); iter != argmap.end(); iter++)
        encoded += urlencode_str(iter->first) + "=" + urlencode_str(iter->second) + "&";

    // Remove the last ampersand.
    return encoded.erase(encoded.size() -1, 1);
}


// For get requests
Request::Request(const std::string &uri) : m_uri(uri)
{
    init(uri);
    m_type = GET;
}

// For post requests.
Request::Request(const std::string &uri, const std::map<std::string, std::string> &postdata) : m_uri(uri), m_data(postdata)
{
    init(uri);
    m_type = POST;
    m_payload = urlencode(postdata);
    m_headers["Content-type"] = "application/x-www-form-urlencoded";
    std::ostringstream len;
    len << m_payload.length();
    m_headers["Content-length"] = len.str();
}

Request::~Request() {}

void Request::addheader(const std::string &key, const std::string &value)
{
    m_headers[key] = value;
}

void Request::init(const std::string &uri)
{
    m_headers["User-Agent"] = "C++ urllib/v0.0.1";
    m_headers["Accept"] = "*/*";

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



std::string Request::getheader(const std::string &key)
{
    // FIXME: make sure that header exists before returning it. Right now this
    // will create an empty entry in the header map if the key doesn't exist.

    return m_headers[key];
}

// Really retardedly basic version of python's urllib2.urlopen
std::string urlopen(Request &req)
{
    using boost::asio::ip::tcp;
    using boost::format;

    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);

    tcp::resolver::query query(req.host(), req.protocol());

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


    // We have a connection!
    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    switch (req.type())
    {
        case GET:
            request_stream << format("GET %s HTTP/1.0\r\n") % req.path();
            break;

        case POST:
            request_stream << format("POST %s HTTP/1.0\r\n") % req.path();
            break;
        
        default:
            throw std::runtime_error("Not implemented.");
    }

    request_stream << format("Host: %s\r\n") % req.host();

    std::map<std::string, std::string> headers = req.headers();

    for (std::map<std::string, std::string>::iterator iter = headers.begin(); iter != headers.end(); iter++)
        request_stream << format("%s: %s\r\n") % iter->first % iter->second;

    request_stream << "Connection: close\r\n\r\n";

    if (req.type() == POST)
    {
        request_stream << format("%s\r\n") % req.payload();
    }
        

    //std::cout << format("DEBUG\n%s\nEND DEBUG\n") % &request;

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
        std::stringstream error_string;
        error_string << format("Response returned with status code %s") % status_code;
        return error_string.str();
    }

    std::ostringstream response_string;

    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;

    // TODO: add parsing headers, sticking them into a response object.
    // For now we just toss them away.
    while (std::getline(response_stream, header) && header != "\r") {}

    boost::asio::streambuf::const_buffers_type bufs = response.data();

#if 0
    // Write whatever content we already have to output.
    if (response.size() > 0)
    {
        response_string.append(bufs);

        //std::cout << &response;
    }
#endif 

    // Read until EOF, writing data to output as we go.
    while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
        response_string << &response;

    if (error != boost::asio::error::eof)
      throw boost::system::system_error(error);

    return response_string.str();
}

};

// to test functionality.
#if 1
int main(int argc, char **argv)
{
    using namespace std;
    using boost::format;
    map<string, string> poast;
    poast["foo"] = "bar";

    httplib::Request req("http://dev.local.lan/debug/", poast);
    cout << httplib::urlopen(req) << endl;

    return 0;
}
#endif 
