// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <string>
#include <c4m/capture_data.hpp>
#include <boost/asio.hpp>

namespace ba = boost::asio;

namespace
{

int check_response(std::istream& response_stream)
{
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;

    std::string status_message;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "RTSP/")
    {
        std::cout << "Invalid response" << std::endl <<  http_version << std::endl;
        return 1;
    }

    if (status_code != 200)
    {
        std::cout << "Response returned with status code " << status_code << "\n";
        return 1;
    }
    return 0;
}


uint8_t get_bit(uint8_t byte, uint8_t location)
{
    assert(location < 8);
    location = 7 - location;
    return ((byte >> location) & 0x1);
}

bool extract_h264(const uint8_t* data, uint32_t size, std::vector<uint8_t>& result, uint32_t& timestamp)
{
    uint32_t byte_offset = 0;

    {
        uint8_t byte = data[byte_offset];
        byte_offset += 1;

        uint8_t extension = get_bit(byte, 3);

        uint8_t csrc_count =
            (get_bit(byte, 4) << 3) |
            (get_bit(byte, 5) << 2) |
            (get_bit(byte, 6) << 1) |
            (get_bit(byte, 7) << 0);

        byte_offset += 3;

        timestamp =
            (uint32_t) data[byte_offset + 0] << 24 |
            (uint32_t) data[byte_offset + 1] << 16 |
            (uint32_t) data[byte_offset + 2] << 8 |
            (uint32_t) data[byte_offset + 3];

        byte_offset += 4;

        byte_offset += 4;

        for (uint8_t i = 0; i < csrc_count; ++i)
        {
            byte_offset += 4;
        }

        if (extension == 1)
        {
            byte_offset += 2;
            uint16_t header_length =
                (uint16_t) data[byte_offset] << 8 |
                (uint16_t) data[byte_offset + 1];

            byte_offset += 2;
            byte_offset += 4 * header_length;
        }
    }
    uint8_t nalu;
    uint8_t nalu_type;
    {
        uint8_t byte = data[byte_offset];

        nalu =
            (get_bit(byte, 0) << 7) |
            (get_bit(byte, 1) << 6) |
            (get_bit(byte, 2) << 5);

        nalu_type =
            (get_bit(byte, 3) << 4) |
            (get_bit(byte, 4) << 3) |
            (get_bit(byte, 5) << 2) |
            (get_bit(byte, 6) << 1) |
            (get_bit(byte, 7) << 0);

        if (nalu_type == 7 /*SPS_TYPE_ID*/ || nalu_type == 8 /*PPS_TYPE_ID*/)
        {
            result.push_back(0);
            result.push_back(0);
            result.push_back(0);
            result.push_back(1);
            for (uint32_t i = byte_offset; i < size; ++i)
            {
                result.push_back(data[i]);
            }
            return true;
        }

        byte_offset += 1;
    }
    bool start;
    {
        uint8_t byte = data[byte_offset];
        byte_offset += 1;
        start = get_bit(byte, 0);

        nalu |=
            (get_bit(byte, 3) << 4) |
            (get_bit(byte, 4) << 3) |
            (get_bit(byte, 5) << 2) |
            (get_bit(byte, 6) << 1) |
            (get_bit(byte, 7) << 0);
    }

    if (nalu_type != 28)
    {
        std::cout << "not FU_A_TYPE " << (uint32_t)nalu_type << std::endl;
        return false;
    }

    if (start)
    {
        result.push_back(0);
        result.push_back(0);
        result.push_back(0);
        result.push_back(1);
        result.push_back(nalu);
    }

    for (uint32_t i = byte_offset; i < size; ++i)
    {
        result.push_back(data[i]);
    }
    return start;
}
}
class rtp_camera
{
public:
    rtp_camera(ba::io_service& io_service):
        m_tcp_socket(io_service),
        m_udp_socket(io_service),
        m_resolver(io_service)
    { }

    void try_open(const std::string& ip, const std::string& port)
    {
        m_ip = ip;
        m_port = port;

        ba::ip::tcp::resolver::query query(m_ip, m_port);
        ba::ip::tcp::resolver::iterator endpoint_iterator =
            m_resolver.resolve(query);
        ba::ip::tcp::resolver::iterator end;


        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end)
        {
            m_tcp_socket.close();
            m_tcp_socket.connect(*endpoint_iterator++, error);
        }

        {
            ba::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "OPTIONS rtsp://" << m_ip << ":" << m_port << "/rtpstream/config5=u RTSP/1.0\r\n";
            request_stream << "CSeq: 2\r\n";
            request_stream << "User-Agent: c4m\r\n";
            request_stream << "\r\n";

            ba::write(m_tcp_socket, request);

            ba::streambuf response;
            ba::read_until(m_tcp_socket, response, "\r\n");
            std::istream response_stream(&response);
            if (check_response(response_stream) != 0)
                throw std::system_error();
        }
        {
            ba::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "DESCRIBE rtsp://" << m_ip << ":" << m_port << "/rtpstream/config5=u RTSP/1.0\r\n";
            request_stream << "CSeq: 3\r\n";
            request_stream << "User-Agent: c4m\r\n";
            request_stream << "\r\n";

            ba::write(m_tcp_socket, request);

            ba::streambuf response;
            ba::read_until(m_tcp_socket, response, "\r\n");
            std::istream response_stream(&response);
            if (check_response(response_stream) != 0)
                throw std::system_error();

            // Presentation description, typically in SDP format
            ba::streambuf throwaway;
            ba::read_until(m_tcp_socket, throwaway, "\r\n");
        }

        {
            ba::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "SETUP rtsp://" << m_ip << "/rtpstream/config5/AVCESEnc RTSP/1.0\r\n";
            request_stream << "CSeq: 4\r\n";
            request_stream << "User-Agent: c4m\r\n";
            request_stream << "Transport: RTP/AVP;unicast;client_port=33096-33097\r\n";
            request_stream << "\r\n";

            ba::write(m_tcp_socket, request);

            ba::streambuf response;
            ba::read_until(m_tcp_socket, response, "\r\n");
            std::istream response_stream(&response);
            if (check_response(response_stream) != 0)
                throw std::system_error();

            std::stringstream ss;
            ss << response_stream.rdbuf();
            std::string response_str = ss.str();

            {
                std::regex session_id_regex("Session: (\\d+)");
                std::smatch base_match;
                if (std::regex_search(response_str, base_match, session_id_regex)) {
                    if (base_match.size() == 2) {
                        std::ssub_match base_sub_match = base_match[1];
                        m_session = base_sub_match.str();
                    }
                }
            }

            {
                std::regex port_regex("client_port=(\\d+)-(\\d+)");
                std::smatch base_match;
                if (std::regex_search(response_str, base_match, port_regex)) {
                    if (base_match.size() > 2) {
                        std::ssub_match base_sub_match = base_match[1];
                        std::string::size_type sz;   // alias of size_t
                        m_client_port = std::stoi(base_sub_match.str(), &sz);
                    }
                }
            }
            assert(m_client_port != 0);
        }
    }

    void try_start_streaming()
    {
        m_udp_socket.open(ba::ip::udp::v4());
        m_udp_socket.bind(ba::ip::udp::endpoint(ba::ip::address_v4::any(), m_client_port));


        ba::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "PLAY rtsp://" << m_ip << ":" << m_port << "/rtpstream/config5=u RTSP/1.0\r\n";
        request_stream << "CSeq: 5\r\n";
        request_stream << "User-Agent: c4m\r\n";
        request_stream << "Session: " << m_session << "\r\n";
        request_stream << "Range: npt=0.000-\r\n";
        request_stream << "\r\n";

        ba::write(m_tcp_socket, request);

        ba::streambuf response;
        ba::read_until(m_tcp_socket, response, "\r\n");
        std::istream response_stream(&response);
        if (check_response(response_stream) != 0)
            throw std::system_error();

    }

    void try_capture(std::vector<uint8_t>& h264, uint32_t& timestamp)
    {
        std::vector<uint8_t> buffer(4096);
        boost::asio::ip::udp::endpoint server;
        size_t received = m_udp_socket.receive_from(boost::asio::buffer(buffer), server);
        return extract_h264(buffer.data(), received, h264, timestamp);
    }

    void try_request_i_frame_period(uint32_t period)
    {
        (void)period;
    }

    void try_request_bitrates(uint32_t average_bitrate, uint32_t peak_bitrate)
    {
        (void)average_bitrate;
        (void)peak_bitrate;
    }

    uint32_t height() const
    {
        return 0;
    }

    uint32_t width() const
    {
        return 0;
    }

private:

    ba::ip::tcp::socket m_tcp_socket;
    ba::ip::udp::socket m_udp_socket;
    ba::ip::tcp::resolver m_resolver;

    uint16_t m_client_port = 0;
    std::string m_session;
    std::string m_ip;
    std::string m_port;
};
