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

    keyFieldCount = setFieldTypeToValue<PbCppType::CPPTYPE_INT32>(&out,&PbReflection::SetInt32,-1);

    if(keyFieldCount != expectedKeyFieldCount)
    {
        throw std::runtime_error(std::string{__PRETTY_FUNCTION__}+" Invalid init for: "+out.descriptor()->full_name());
    }

    return out;
}

} // namespace tc
