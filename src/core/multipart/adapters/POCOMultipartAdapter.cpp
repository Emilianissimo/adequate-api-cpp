//
// Created by Emil Erofeevskiy on 11/01/26.
//

#include "../POCOMultipartAdapter.h"
#include <Poco/Net/MediaType.h>
#include <Poco/Net/MultipartReader.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/StreamCopier.h>
#include <Poco/Exception.h>

#include "core/errors/Errors.h"

std::vector<Part> POCOMultipartAdapter::parse(const std::string& contentType, const std::string& body) const {
    try {
        Poco::Net::MediaType mt(contentType);

        if (mt.getType() != "multipart")
            throw MultipartError("Content-Type is not multipart/*: " + contentType);

        std::string boundary;
        if (mt.hasParameter("boundary")) boundary = mt.getParameter("boundary");
        if (boundary.empty())
            throw MultipartError("Multipart boundary is missing in Content-Type: " + contentType);

        std::istringstream bodyStream(body);
        Poco::Net::MultipartReader reader(bodyStream, boundary);

        std::vector<Part> parts;

        while (reader.hasNextPart()) {
            Poco::Net::MessageHeader h;
            reader.nextPart(h);
            std::istream& ps = reader.stream();

            Part part{};

            // capture headers
            for (const auto&[fst, snd] : h) {
                part.headers[fst] = snd;
            }

            part.contentType = headerGet(h, "Content-Type");

            // Content-Disposition -> name/filename
            const std::string cd = headerGet(h, "Content-Disposition");
            if (cd.empty())
                throw MultipartError("Multipart part missing Content-Disposition");

            // строго проверяем первый токен
            if (const auto first = trimCopy(cd.substr(0, cd.find(';'))); first != "form-data")
                throw MultipartError("Unsupported Content-Disposition: " + cd);

            auto params = parseHeaderParams(cd);

            auto itName = params.find("name");
            if (itName == params.end() || itName->second.empty())
                throw MultipartError("Multipart part missing disposition parameter: name");

            part.name = itName->second;

            if (auto it = params.find("filename"); it != params.end())
                part.filename = it->second;

            part.name = itName->second;

            if (auto it = params.find("filename"); it != params.end())
                part.filename = it->second;

            // read body bytes of this part
            std::ostringstream out;
            Poco::StreamCopier::copyStream(ps, out);
            part.data = out.str();

            parts.emplace_back(std::move(part));
        }
        return parts;
    } catch (const Poco::Exception& ex) {
        throw MultipartError("Poco multipart parse failed: " + ex.displayText());
    } catch (const std::exception& ex) {
        throw MultipartError("Multipart parse failed: " + std::string(ex.what()));
    }
}

std::string POCOMultipartAdapter::headerGet(const Poco::Net::MessageHeader& h, const std::string& key) {
    return h.has(key) ? h.get(key) : "";
}

std::string POCOMultipartAdapter::trimCopy(std::string s) {
    auto isSpace = [](const unsigned char c) { return std::isspace(c) != 0;};
    while (!s.empty() && isSpace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && isSpace(static_cast<unsigned char>(s.back()))) s.pop_back();
    return s;
}

std::string POCOMultipartAdapter::unquote(std::string s) {
    s = trimCopy(std::move(s));
    if (s.size() >= 2) {
        const char a = s.front();
        const char b = s.back();
        if ((a == '"' && b == '"') || (a == '\'' && b == '\'')) {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}

std::unordered_map<std::string, std::string> POCOMultipartAdapter::parseHeaderParams(const std::string& headerValue) {
    std::unordered_map<std::string, std::string> out;

    size_t start = 0;
    bool firstToken = true;

    while (start < headerValue.size()) {
        size_t end = headerValue.find(';', start);
        if (end == std::string::npos) end = headerValue.size();

        std::string token = trimCopy(headerValue.substr(start, end - start));
        start = end + 1;

        if (token.empty()) continue;

        if (firstToken) {
            firstToken = false;
            continue;
        }

        const size_t eq = token.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trimCopy(token.substr(0, eq));
        std::string val = unquote(token.substr(eq + 1));

        if (!key.empty()) out[key] = val;
    }
    return out;
}
