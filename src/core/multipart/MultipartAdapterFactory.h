//
// Created by Emil Erofeevskiy on 11/01/26.
//

#ifndef BEAST_API_MULTIPARTADAPTERFACTORY_H
#define BEAST_API_MULTIPARTADAPTERFACTORY_H

#include "string"
#include "core/multipart/adapters/POCOMultipartAdapter.h"
#include "interfaces/MultipartAdapterInterface.h"

using MultipartParseFn =
    std::vector<Part>(*)(const std::string&, const std::string&);

class MultipartAdapterFactory {
public:
    static std::unique_ptr<MultipartAdapterInterface>
    create(const std::string& name) {
        if (name == "POCO")
            return std::make_unique<POCOMultipartAdapter>();

        throw std::logic_error("Unsupported adapter: " + name);
    }
};

#endif //BEAST_API_MULTIPARTADAPTERFACTORY_H