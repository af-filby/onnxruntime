// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "core/common/inlined_containers.h"
#include "core/platform/env.h"
#include "core/framework/ort_value.h"

#include "lora/lora_format_utils.h"

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

namespace onnxruntime {
namespace lora {

struct Adapter;

namespace details {
// This class takes hold of the serialized parameters that
// are either loaded from disk or mapped from disk (coming in the future)
// This data is always in host memory.
class BinaryFormatHolder {
 public:
  BinaryFormatHolder() = default;
  BinaryFormatHolder(const BinaryFormatHolder&) = delete;
  BinaryFormatHolder& operator=(const BinaryFormatHolder&) = delete;
  ~BinaryFormatHolder();

  BinaryFormatHolder(BinaryFormatHolder&&) = default;
  BinaryFormatHolder& operator=(BinaryFormatHolder&&) = default;

  /// <summary>
  /// Load parameters from an adapter file and validates its format.
  /// </summary>
  /// <param name="file_name">file name that can be opened</param>
  void Load(const std::filesystem::path& file_path);

  /// <summary>
  /// Memory maps adapter file into memory and validates its format.
  /// </summary>
  /// <param name="file_name"></param>
  void MemoryMap(const std::filesystem::path& file_path);

  // Get Flatbuffer object pointer
  const Adapter* GetBinaryAdapter() const noexcept { return adapter_; }

  // Get the size of the buffer
  size_t GetSize() const;

 private:
  struct BufferHolder {
    explicit BufferHolder(std::vector<uint8_t> buffer) : buffer_(std::move(buffer)) {}
    std::vector<uint8_t> buffer_;
  };

  struct MemMapHolder {
    MemMapHolder(Env::MappedMemoryPtr mapped_memory, size_t file_size)
        : mapped_memory_(std::move(mapped_memory)), file_size_(file_size) {}
    Env::MappedMemoryPtr mapped_memory_;
    size_t file_size_;
  };

  std::variant<MemMapHolder, BufferHolder> buffer_;
  const Adapter* adapter_{nullptr};
};

/// <summary>
/// Represents a named lora parameter (tensor)
/// </summary>
struct LoraParam {
  LoraParam() = default;
  LoraParam(std::string name, OrtValue parameter);

  std::string name_;
  OrtValue ort_value_;
};

}  // namespace details

/// <summary>
/// Container to hold and access Lora Parameters
/// </summary>
class LoraAdapter {
 public:
  LoraAdapter() = default;
  LoraAdapter(const LoraAdapter&) = delete;
  LoraAdapter& operator=(const LoraAdapter&) = delete;
  ~LoraAdapter() = default;

  LoraAdapter(LoraAdapter&&) = default;
  LoraAdapter& operator=(LoraAdapter&&) = default;

  /// <summary>
  /// Load parameters into memory from an adapter file and validates its format.
  /// </summary>
  /// <param name="file_name">file name that can be opened</param>
  void Load(const std::filesystem::path& file_path);

  /// <summary>
  /// Memory maps adapter file into memory and validates its format.
  /// </summary>
  /// <param name="file_name"></param>
  void MemoryMap(const std::filesystem::path& file_path);

  template <class NamesOutputIter, class TensorOutputIter>
  void OutputAdaptersParameters(NamesOutputIter names_out,
                                TensorOutputIter params_out) {
    const auto* adapter = binary_format_holder_.GetBinaryAdapter();
    utils::OutputAdaptersParameters(*adapter, names_out, params_out);
  }

 private:
  details::BinaryFormatHolder binary_format_holder_;
};

}  // namespace lora
}  // namespace onnxruntime
