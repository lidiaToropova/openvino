// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <inference_engine.hpp>

using namespace InferenceEngine;

/**
 * @brief Determine if InferenceEngine blob means image or not
 */
template<typename T>
static bool isImage(const T &blob) {
    auto descriptor = blob->getTensorDesc();
    if (descriptor.getLayout() != InferenceEngine::NCHW) {
        return false;
    }
    auto channels = descriptor.getDims()[1];
    return channels == 3;
}


/**
 * @brief Determine if InferenceEngine blob means image information or not
 */
template<typename T>
static bool isImageInfo(const T &blob) {
    auto descriptor = blob->getTensorDesc();
    if (descriptor.getLayout() != InferenceEngine::NC) {
        return false;
    }
    auto channels = descriptor.getDims()[1];
    return (channels >= 2);
}


/**
 * @brief Return height and width from provided InferenceEngine tensor description
 */
inline std::pair<size_t, size_t> getTensorHeightWidth(const InferenceEngine::TensorDesc& desc) {
    const auto& layout = desc.getLayout();
    const auto& dims = desc.getDims();
    const auto& size = dims.size();
    if ((size >= 2) &&
        (layout == InferenceEngine::Layout::NCHW  ||
         layout == InferenceEngine::Layout::NHWC  ||
         layout == InferenceEngine::Layout::NCDHW ||
         layout == InferenceEngine::Layout::NDHWC ||
         layout == InferenceEngine::Layout::OIHW  ||
         layout == InferenceEngine::Layout::GOIHW ||
         layout == InferenceEngine::Layout::OIDHW ||
         layout == InferenceEngine::Layout::GOIDHW ||
         layout == InferenceEngine::Layout::CHW  ||
         layout == InferenceEngine::Layout::HW)) {
        // Regardless of layout, dimensions are stored in fixed order
        return std::make_pair(dims.back(), dims.at(size - 2));
    } else {
        throw std::logic_error("Tensor does not have height and width dimensions");
    }
}


/**
 * @brief Fill InferenceEngine blob with random values
 */
template<typename T>
void fillBlobRandom(Blob::Ptr& inputBlob) {
    MemoryBlob::Ptr minput = as<MemoryBlob>(inputBlob);
    // locked memory holder should be alive all time while access to its buffer happens
    auto minputHolder = minput->wmap();

    auto inputBlobData = minputHolder.as<T *>();
    for (size_t i = 0; i < inputBlob->size(); i++) {
        auto rand_max = RAND_MAX;
        inputBlobData[i] = (T) rand() / static_cast<T>(rand_max) * 10;
    }
}

/**
 * @brief Fill InferenceEngine tensor with random values
 */
template<typename T>
ov::Tensor fillTensorRandom(const ov::Output<const ov::Node>& input) {
    ov::Tensor tensor { input.get_element_type(), input.get_shape() };
    std::vector<T>values(ov::shape_size(input.get_shape()));
    for (size_t i = 0; i < values.size(); ++i) {
        auto rand_max = RAND_MAX;
        values[i] = (T) rand() / static_cast<T>(rand_max) * 10;
    }
    std::memcpy(tensor.data(), values.data(), sizeof(T) * values.size());
    return tensor;
}


/**
 * @brief Fill InferenceEngine blob with image information
 */
template<typename T>
void fillBlobImInfo(Blob::Ptr& inputBlob,
                    const size_t& batchSize,
                    std::pair<size_t, size_t> image_size) {
    MemoryBlob::Ptr minput = as<MemoryBlob>(inputBlob);
    // locked memory holder should be alive all time while access to its buffer happens
    auto minputHolder = minput->wmap();

    auto inputBlobData = minputHolder.as<T *>();
    for (size_t b = 0; b < batchSize; b++) {
        size_t iminfoSize = inputBlob->size()/batchSize;
        for (size_t i = 0; i < iminfoSize; i++) {
            size_t index = b*iminfoSize + i;
            if (0 == i)
                inputBlobData[index] = static_cast<T>(image_size.first);
            else if (1 == i)
                inputBlobData[index] = static_cast<T>(image_size.second);
            else
                inputBlobData[index] = 1;
        }
    }
}

/**
 * @brief Fill InferenceEngine tensor with random values
 */
template<typename T>
ov::Tensor fillTensorImInfo(ov::Output<const ov::Node>& input,
                            std::pair<size_t, size_t> image_size) {
    ov::Tensor tensor {input.get_element_type(), input.get_shape()};
    std::vector<float> values{static_cast<float>(image_size.first), static_cast<float>(image_size.second)};
    std::memcpy(tensor.data(), values.data(), sizeof(T) * values.size());
    return tensor;
}


/**
 * @brief Fill InferRequest blobs with random values or image information
 */
void fillBlobs(InferenceEngine::InferRequest inferRequest,
               const InferenceEngine::ConstInputsDataMap& inputsInfo,
               const size_t& batchSize);

/**
 * @brief Fill InferRequest tensors with random values or image information
 */
void fillTensors(ov::InferRequest &infer_request,
                 std::vector<ov::Output<const ov::Node>> inputs);