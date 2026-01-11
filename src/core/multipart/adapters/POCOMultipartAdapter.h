//
// Created by Emil Erofeevskiy on 11/01/26.
//

#ifndef BEAST_API_MULTIPARTADAPTER_H
#define BEAST_API_MULTIPARTADAPTER_H

#include <sstream>
#include <Poco/Net/MessageHeader.h>

#include "core/interfaces/MultipartAdapterInterface.h"


class POCOMultipartAdapter final : public MultipartAdapterInterface {
public:
    [[nodiscard]] std::vector<Part> parse(const std::string& contentType, const std::string& body) const override;

private:
    static std::string headerGet(const Poco::Net::MessageHeader& h, const std::string& key);

    static std::string trimCopy(std::string s);

    static std::string unquote(std::string s);

    // "form-data; name=\"file\"; filename=\"a.txt\"" -> {name:file, filename:a.txt}
    static std::unordered_map<std::string, std::string> parseHeaderParams(const std::string& headerValue);
};


#endif //BEAST_API_MULTIPARTADAPTER_H