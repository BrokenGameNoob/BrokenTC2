#pragma once

#include "TCprofile.pb.h"

#include "ProtoHelp.hpp" //NOLINT

namespace tc {

template<FromPbMessage Proto_t>
inline
int32_t initKeysToDefault(Proto_t *msg, auto target_key){ // is_keyboard_key || is_controller_key
    int32_t match_count{};

    std::function<bool(const google::protobuf::FieldDescriptor& desc, const google::protobuf::Reflection& reflection)> to_apply =
        [&msg,&match_count](const google::protobuf::FieldDescriptor& desc, const google::protobuf::Reflection& reflection)->bool{

        auto kIsControllerKey{tc::GetFieldOption<Proto_t>(desc.number(),tc::is_controller_key)};
        if(!kIsControllerKey){
            return false;
        }
        ++match_count;
        reflection.SetInt32(msg,&desc,constants::kUnbindValue);

        return false;
    };
    tc::applyOnPbFields(msg,to_apply);
    return match_count;
}

//https://protobuf.dev/reference/cpp/api-docs/google.protobuf.message/#Reflection
template<>
ControllerProfile getDefault(){
    ControllerProfile out{};

    out.set_profilename("Default");

    constexpr int32_t kExpectedKeyFieldCount{11};

    const int32_t kKeyFieldCount{initKeysToDefault(&out, tc::is_controller_key)};

    if(kKeyFieldCount != kExpectedKeyFieldCount)
    {
        qDebug() << "KeyFieldCount:" << kKeyFieldCount;
        qDebug() << "Expected:" << kExpectedKeyFieldCount;
        throw std::runtime_error(std::string{__PRETTY_FUNCTION__}+" Invalid init for: "+out.descriptor()->full_name());
    }

    return out;
}

template<>
GameProfile getDefault(){
    GameProfile out{};

    out.set_profilename("Default");

    constexpr int32_t kExpectedKeyFieldCount{11};

    const int32_t kKeyFieldCount{initKeysToDefault(&out)};

    if(kKeyFieldCount != kExpectedKeyFieldCount)
    {
        qDebug() << "KeyFieldCount:" << kKeyFieldCount;
        qDebug() << "Expected:" << kExpectedKeyFieldCount;
        throw std::runtime_error(std::string{__PRETTY_FUNCTION__}+" Invalid init for: "+out.descriptor()->full_name());
    }

    return out;
}

} // namespace tc
