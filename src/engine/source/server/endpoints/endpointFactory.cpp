/* Copyright (C) 2015-2021, Wazuh Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "endpointFactory.hpp"

#include <memory>
#include <stdexcept>
#include <string>

#include "baseEndpoint.hpp"
//#include "socketEndpoint.hpp"
#include "tcpEndpoint.hpp"

using namespace std;

namespace engineserver::endpoints
{

EndpointType stringToEndpoint(const string & endpointName)
{
    if (endpointName == "tcp")
    {
        return TCP;
    }
    // else if (endpointName == "socket")
    // {
    //     return SOCKET;
    // }
    else
    {
        throw invalid_argument("Error, endpoint " + endpointName + " not supported");
    }
}

unique_ptr<BaseEndpoint> create(const string & type, const string & config,
                                moodycamel::BlockingConcurrentQueue<std::string> & eventBuffer)
{
    auto endpointType = stringToEndpoint(type);
    switch (endpointType)
    {
        case TCP:
            return make_unique<TCPEndpoint>(config, eventBuffer);
            break;
        // case SOCKET:
        //     return make_unique<SocketEndpoint>(config);
        //     break;
        default:
            throw runtime_error("Error, endpoint type " + to_string(endpointType) +
                                " not implemented by factory Endpoint builder");
            break;
    }
}

} // namespace engineserver::endpoints