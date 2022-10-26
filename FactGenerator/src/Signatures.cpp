#include "Signatures.hpp"

template <typename T>
auto extract_from_array(const llvm::json::Array &json_array, size_t index)
    -> llvm::Optional<T>;

template <>
auto extract_from_array(const llvm::json::Array &json_array, size_t index)
    -> llvm::Optional<int64_t> {
  return json_array[index].getAsInteger();
}

template <>
auto extract_from_array(const llvm::json::Array &json_array, size_t index)
    -> llvm::Optional<std::string> {
  auto val = json_array[index].getAsString();
  if (val.hasValue()) {
    return {"@" + val.getValue().str()};
  }
  return {};
}

template <typename T, typename... Ts>
auto extract_many_from_array(const llvm::json::Array &json_array, size_t index)
    -> std::tuple<T, Ts...> {
  auto t = extract_from_array<T>(json_array, index);
  if (!t.hasValue()) {
    throw std::invalid_argument("Wrong type of argument for signature!");
  }
  if constexpr (sizeof...(Ts) == 0) {
    return std::make_tuple(t.getValue());
  } else {  // NOLINT: clang-tidy doesn't know about "if constexpr"
    return std::tuple_cat(
        std::make_tuple(t.getValue()),
        extract_many_from_array<Ts...>(json_array, index + 1));
  }
}

template <typename... Ts>
void do_write(
    cclyzer::ForwardingFactWriter &writer,
    const std::string &function_name,
    const cclyzer::predicates::pred_t &pred,
    Ts... args) {
  writer.writeFact(pred, "@" + function_name, args...);
}

template <typename... Ts>
void write_pts_signature(
    const cclyzer::predicates::pred_t &pred,
    const std::string &signature_type,
    const std::string &function_name,
    cclyzer::ForwardingFactWriter &writer,
    const llvm::json::Value &signature_component_value) {
  const auto *signature_component_array =
      signature_component_value.getAsArray();
  if (signature_component_array == nullptr) {
    throw std::invalid_argument(
        "Signature " + signature_type + " expects an array of arguments");
  }
  if (signature_component_array->size() != sizeof...(Ts)) {
    throw std::invalid_argument(
        "Expected " + std::to_string(sizeof...(Ts)) +
        " arguments for signature " + signature_type + " but found " +
        std::to_string(signature_component_array->size()));
  }
  // It's hard to write the following generically over size :'(
  if constexpr (sizeof...(Ts) == 1) {
    const auto result =
        extract_many_from_array<Ts...>(*signature_component_array, 0);
    writer.writeFact(pred, "@" + function_name, std::get<0>(result));
  } else if constexpr (sizeof...(Ts) == 2) {
    const auto result =
        extract_many_from_array<Ts...>(*signature_component_array, 0);
    writer.writeFact(
        pred, "@" + function_name, std::get<0>(result), std::get<1>(result));
  } else {
    static_assert(sizeof...(Ts) == 0);
    writer.writeFact(pred, "@" + function_name);
  }
}

auto preprocess_signatures(const boost::filesystem::path &signatures_path)
    -> std::vector<std::tuple<std::string, std::regex, llvm::json::Array>> {
  std::vector<std::tuple<std::string, std::regex, llvm::json::Array>>
      functions_with_signatures;
  std::ifstream ifs(signatures_path);
  std::string content(
      (std::istreambuf_iterator<char>(ifs)),
      (std::istreambuf_iterator<char>()));
  std::string error_message("Invalid signatures file: ");
  error_message.append(signatures_path.string());

  auto maybe_value = llvm::json::parse(content);
  if (!maybe_value) {
    error_message.append(" was not valid JSON");
    throw std::invalid_argument(error_message);
  }

  const auto *obj = maybe_value.get().getAsObject();
  if (obj == nullptr) {
    error_message.append(" was not a JSON object");
    throw std::invalid_argument(error_message);
  }

  for (const auto &[function_name_key, function_value] : *obj) {
    const auto &function_name = function_name_key.str();
    error_message.append(" had an invalid signature for ");
    error_message.append(function_name);

    const auto *function_array = function_value.getAsArray();
    if (function_array == nullptr) {
      throw std::invalid_argument(error_message);
    }
    functions_with_signatures.emplace_back(
        function_name, function_name, *function_array);
  }
  return functions_with_signatures;
}

void emit_signatures(
    cclyzer::ForwardingFactWriter &writer,
    const std::string &function_name,
    const llvm::json::Array &signatures) {
  std::string error_message(
      "Invalid signature for function " + function_name + ": ");
  for (const auto &signature_component_value : signatures) {
    const auto *signature_component = signature_component_value.getAsObject();
    if (signature_component == nullptr || signature_component->size() != 1) {
      throw std::invalid_argument(error_message);
    }

    const auto
        &[signature_component_type_key, signature_component_inner_value] =
            *signature_component->begin();
    const auto &signature_component_type = signature_component_type_key.str();

    if (signature_component_type == "dataflow" ||
        signature_component_type == "output" ||
        signature_component_type == "input") {
      continue;
    }
    if (signature_component_type == "pts_none") {
      write_pts_signature<>(
          cclyzer::predicates::signature::none,
          "pts_none",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_return_alloc") {
      write_pts_signature<>(
          cclyzer::predicates::signature::return_alloc,
          "pts_return_alloc",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_return_alloc_once") {
      write_pts_signature<>(
          cclyzer::predicates::signature::return_alloc_once,
          "pts_return_alloc_once",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_return_aliases_arg") {
      write_pts_signature<int64_t>(
          cclyzer::predicates::signature::return_aliases_arg,
          "pts_return_aliases_arg",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_return_aliases_arg_reachable") {
      write_pts_signature<int64_t>(
          cclyzer::predicates::signature::return_aliases_arg_reachable,
          "pts_return_aliases_arg_reachable",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_return_points_to_global") {
      write_pts_signature<std::string>(
          cclyzer::predicates::signature::return_points_to_global,
          "pts_return_points_to_global",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_return_aliases_global") {
      write_pts_signature<std::string>(
          cclyzer::predicates::signature::return_aliases_global,
          "pts_return_aliases_global",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (
        signature_component_type == "pts_return_aliases_global_reachable") {
      write_pts_signature<std::string>(
          cclyzer::predicates::signature::return_aliases_global_reachable,
          "pts_return_aliases_global_reachable",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_arg_alloc") {
      write_pts_signature<int64_t>(
          cclyzer::predicates::signature::arg_alloc,
          "pts_arg_alloc",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_arg_alloc_once") {
      write_pts_signature<int64_t>(
          cclyzer::predicates::signature::arg_alloc_once,
          "pts_arg_alloc_once",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_arg_memcpy_arg") {
      write_pts_signature<int64_t, int64_t>(
          cclyzer::predicates::signature::arg_memcpy_arg,
          "pts_arg_memcpy_arg",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_arg_memcpy_arg_reachable") {
      write_pts_signature<int64_t, int64_t>(
          cclyzer::predicates::signature::arg_memcpy_arg_reachable,
          "pts_arg_memcpy_arg_reachable",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_arg_points_to_global") {
      write_pts_signature<int64_t, std::string>(
          cclyzer::predicates::signature::arg_points_to_global,
          "pts_arg_points_to_global",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_arg_memcpy_global") {
      write_pts_signature<int64_t, std::string>(
          cclyzer::predicates::signature::arg_memcpy_global,
          "pts_arg_memcpy_global",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_arg_memcpy_global_reachable") {
      write_pts_signature<int64_t, std::string>(
          cclyzer::predicates::signature::arg_memcpy_global_reachable,
          "pts_arg_memcpy_global_reachable",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_global_memcpy_arg") {
      write_pts_signature<std::string, int64_t>(
          cclyzer::predicates::signature::global_memcpy_arg,
          "pts_global_memcpy_arg",
          function_name,
          writer,
          signature_component_inner_value);
    } else if (signature_component_type == "pts_global_memcpy_arg_reachable") {
      write_pts_signature<std::string, int64_t>(
          cclyzer::predicates::signature::global_memcpy_arg_reachable,
          "pts_global_memcpy_arg_reachable",
          function_name,
          writer,
          signature_component_inner_value);
    } else {
      error_message.append(": unknown signature type '");
      error_message.append(signature_component_type + "'");
      throw std::invalid_argument(error_message);
    }
  }
}
