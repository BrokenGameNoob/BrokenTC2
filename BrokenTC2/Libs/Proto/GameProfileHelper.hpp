#pragma once

#include "TCprofile.pb.h"

#include "ProtoHelp.hpp" //NOLINT

namespace tc {

//https://protobuf.dev/reference/cpp/api-docs/google.protobuf.message/#Reflection
template<>
ControllerProfile getDefault(){
    ControllerProfile out{};

    out.set_profilename("Default");

    constexpr auto expectedKeyFieldCount{11};

    int32_t keyFieldCount{0};

    const auto* desc = out.GetDescriptor();
    const auto* ref = out.GetReflection();
    for (int i = 0; i < desc->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field_desc = desc->field(i);
        switch (field_desc->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32://it's a key
            ++keyFieldCount;
            ref->SetInt32(&out,desc->field(i),-1);
            break;
        default:
            break;
        }
    }
    if(keyFieldCount != expectedKeyFieldCount)
    {
        throw std::runtime_error(std::string{__PRETTY_FUNCTION__}+" Invalid init for: "+out.descriptor()->full_name());
    }

    return out;
}

} // namespace tc
