//
// Created by user on 28.02.2026.
//

#ifndef BEAST_API_OPENAPIREGISTRAR_H
#define BEAST_API_OPENAPIREGISTRAR_H

#include "types/OpenApiSchemaRegistry.h"

class OpenApiRegistrar
{
public:
    template <class SerializerT>
    static void registerSchema()
    {
        OpenApiSchemaRegistry::instance().registerSchema(
            OpenApiSchemaSpec<SerializerT>::name(),
            OpenApiSchemaSpec<SerializerT>::schema()
        );
    }
};

#endif //BEAST_API_OPENAPIREGISTRAR_H
