// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "remote_context.hpp"

#include <memory>

#include "openvino/proxy/plugin.hpp"
#include "openvino/runtime/iremote_context.hpp"
#include "openvino/runtime/so_ptr.hpp"
#include "remote_tensor.hpp"

ov::proxy::RemoteContext::RemoteContext(ov::SoPtr<ov::IRemoteContext>&& ctx,
                                        const std::string& dev_name,
                                        size_t dev_index,
                                        bool has_index,
                                        bool is_new_api)
    : m_context(std::move(ctx)) {
    m_tensor_name = dev_name + "." + std::to_string(dev_index);
    // New API always has full name, in legacy API we can have device name without index
    if (is_new_api || has_index)
        m_name = m_tensor_name;
    else
        m_name = dev_name;
}

const std::string& ov::proxy::RemoteContext::get_device_name() const {
    return m_name;
}

const ov::AnyMap& ov::proxy::RemoteContext::get_property() const {
    return m_context->get_property();
}

ov::SoPtr<ov::ITensor> ov::proxy::RemoteContext::wrap_tensor(const ov::SoPtr<ov::ITensor>& tensor) {
    auto proxy_tensor = std::make_shared<ov::proxy::RemoteTensor>(tensor, m_tensor_name);
    return ov::SoPtr<ov::ITensor>(std::dynamic_pointer_cast<ov::ITensor>(proxy_tensor), nullptr);
}

ov::SoPtr<ov::IRemoteTensor> ov::proxy::RemoteContext::create_tensor(const ov::element::Type& type,
                                                                     const ov::Shape& shape,
                                                                     const ov::AnyMap& params) {
    auto proxy_tensor =
        std::make_shared<ov::proxy::RemoteTensor>(m_context->create_tensor(type, shape, params), m_tensor_name);
    return ov::SoPtr<ov::IRemoteTensor>(std::dynamic_pointer_cast<ov::IRemoteTensor>(proxy_tensor), nullptr);
}

ov::SoPtr<ov::ITensor> ov::proxy::RemoteContext::create_host_tensor(const ov::element::Type type,
                                                                    const ov::Shape& shape) {
    return m_context->create_host_tensor(type, shape);
}

const ov::SoPtr<ov::IRemoteContext>& ov::proxy::RemoteContext::get_hardware_context(
    const ov::SoPtr<ov::IRemoteContext>& context) {
    if (auto proxy_context = std::dynamic_pointer_cast<ov::proxy::RemoteContext>(context._ptr)) {
        return proxy_context->m_context;
    }
    return context;
}

const ov::SoPtr<ov::IRemoteContext>& ov::proxy::get_hardware_context(const ov::SoPtr<ov::IRemoteContext>& context) {
    return ov::proxy::RemoteContext::get_hardware_context(context);
}