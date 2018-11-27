/*
 * syslog_js.cpp
 *
 *  Created on: Aug 29, 2018
 *      Author: hhdang
 */
#include <string.h>
#include <string>
#include <sstream>

#include <iostream>
#include <vector>

#include <stdlib.h>
#include <fcgiapp.h>
#include <syslog.h>

#include "fcgi.h"
#include "simplewebfactory.h"
#include "MPFDParser/Parser.h"
#include "MPFDParser/Field.h"
#include "MPFDParser/Exception.h"

static void jsonQuote(std::ostringstream &ss, std::string str)
{
    ss << '"';
    for (char& c : str) {
        switch (c)
        {
        case '\\':
        case '"':
            ss << '\\';
            ss << c;
            break;
        case '/':
            //                if (b == '<') {
            ss << '\\';
            //                }
            ss << c;
            break;
        case '\b':
            ss << "\\b";
            break;
        case '\t':
            ss << "\\t";
            break;
        case '\n':
            ss << "\\n";
            break;
        case '\f':
            ss << "\\f";
            break;
        case '\r':
            ss << "\\r";
            break;
        default:
            ss << c;
        }
    }
    ss << '"';
}

std::vector<std::string> modify_syslog(std::string syslog) {
    std::vector<std::string> new_syslog;

    char * buf = strdup(syslog.c_str());
    char * pch = strtok (buf, "\n");

    while (pch != NULL)
    {
        new_syslog.push_back(pch);
        pch = strtok (NULL, "\n");
    }

    return new_syslog;
}

std::ostringstream filter_syslog(std::vector<std::string> syslog, std::string filter_content) {
    std::ostringstream syslog_filter;
    int size = syslog.size();

    for(int i = 0; i < size; i++)
    {
        if (syslog[i].find(filter_content) != std::string::npos) {
            syslog_filter << syslog[i] << "\n";
        }
    }

    return syslog_filter;
}

std::string json_handle_syslog(FCGX_Request *request)
{
    const char *method      = FCGX_GetParam("REQUEST_METHOD", request->envp);
    const char *contentType = FCGX_GetParam("CONTENT_TYPE", request->envp);
    std::ostringstream ss_json;

    if (method && (strcmp(method, "GET") == 0)) {
        std::string syslog_str;
        ss_json << "{\"syslog\": ";
        if (simpleWebFactory::file_to_string("/tmp/messages", syslog_str)) {
            jsonQuote(ss_json, syslog_str);
        }
        ss_json << "}";
    }

    if (method && (strcmp(method, "POST") == 0) && contentType) {
        std::string syslog_str;
        std::string data;
        std::ostringstream syslog_filter;
        std::vector<std::string> new_syslog;

        if (simpleWebFactory::get_post_data(request, data))
        {
            std::string filter_content;
            try {
                MPFD::Parser POSTParser;

                POSTParser.SetContentType(contentType);

                POSTParser.AcceptSomeData(data.c_str(), data.size());

                filter_content = POSTParser.GetFieldText("filter");

            }catch (MPFD::Exception &e)
            {
                syslog(LOG_ERR, "%s\n", e.GetError().c_str());
            }

            ss_json << "{\"syslog\": ";
            if (simpleWebFactory::file_to_string("/tmp/messages", syslog_str))
            {
                new_syslog = modify_syslog(syslog_str);
                syslog_filter = filter_syslog(new_syslog, filter_content);
                jsonQuote(ss_json, syslog_filter.str());
            }
            ss_json << "}";
        }

    }

    return ss_json.str();
}
