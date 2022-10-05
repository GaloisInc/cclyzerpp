#pragma once

#include <cassert>
#include <iostream>
#include <string>

// For now, we can just hardcode a few context sensitivity options.  Down the
// road we will want something more flexible that will probably involve string
// parsing products of features, like Pidgin, but we need something that just
// works more urgently.
enum ContextSensitivity {
  INSENSITIVE,
  CALLSITE1,
  CALLSITE2,
  CALLSITE3,
  CALLSITE4,
  CALLSITE5,
  CALLSITE6,
  CALLSITE7,
  CALLSITE8,
  CALLSITE9,
  CALLER1,
  CALLER2,
  CALLER3,
  CALLER4,
  CALLER5,
  CALLER6,
  CALLER7,
  CALLER8,
  CALLER9,
};

constexpr char const* INSENSITIVE_STRING = "insensitive";
constexpr char const* CALLSITE1_STRING = "1-callsite";
constexpr char const* CALLSITE2_STRING = "2-callsite";
constexpr char const* CALLSITE3_STRING = "3-callsite";
constexpr char const* CALLSITE4_STRING = "4-callsite";
constexpr char const* CALLSITE5_STRING = "5-callsite";
constexpr char const* CALLSITE6_STRING = "6-callsite";
constexpr char const* CALLSITE7_STRING = "7-callsite";
constexpr char const* CALLSITE8_STRING = "8-callsite";
constexpr char const* CALLSITE9_STRING = "9-callsite";
constexpr char const* CALLER1_STRING = "1-caller";
constexpr char const* CALLER2_STRING = "2-caller";
constexpr char const* CALLER3_STRING = "3-caller";
constexpr char const* CALLER4_STRING = "4-caller";
constexpr char const* CALLER5_STRING = "5-caller";
constexpr char const* CALLER6_STRING = "6-caller";
constexpr char const* CALLER7_STRING = "7-caller";
constexpr char const* CALLER8_STRING = "8-caller";
constexpr char const* CALLER9_STRING = "9-caller";

const std::string INSENSITIVE_STD_STRING = std::string(INSENSITIVE_STRING);
const std::string CALLSITE1_STD_STRING = std::string(CALLSITE1_STRING);
const std::string CALLSITE2_STD_STRING = std::string(CALLSITE2_STRING);
const std::string CALLSITE3_STD_STRING = std::string(CALLSITE3_STRING);
const std::string CALLSITE4_STD_STRING = std::string(CALLSITE4_STRING);
const std::string CALLSITE5_STD_STRING = std::string(CALLSITE5_STRING);
const std::string CALLSITE6_STD_STRING = std::string(CALLSITE6_STRING);
const std::string CALLSITE7_STD_STRING = std::string(CALLSITE7_STRING);
const std::string CALLSITE8_STD_STRING = std::string(CALLSITE8_STRING);
const std::string CALLSITE9_STD_STRING = std::string(CALLSITE9_STRING);
const std::string CALLER1_STD_STRING = std::string(CALLER1_STRING);
const std::string CALLER2_STD_STRING = std::string(CALLER2_STRING);
const std::string CALLER3_STD_STRING = std::string(CALLER3_STRING);
const std::string CALLER4_STD_STRING = std::string(CALLER4_STRING);
const std::string CALLER5_STD_STRING = std::string(CALLER5_STRING);
const std::string CALLER6_STD_STRING = std::string(CALLER6_STRING);
const std::string CALLER7_STD_STRING = std::string(CALLER7_STRING);
const std::string CALLER8_STD_STRING = std::string(CALLER8_STRING);
const std::string CALLER9_STD_STRING = std::string(CALLER9_STRING);

// This needs to be kept in sync with user_options.dl.
constexpr auto context_sensitivity_to_string(
    const ContextSensitivity& sensitivity) -> char const* {
  switch (sensitivity) {
    case CALLSITE1:
      return CALLSITE1_STRING;
      break;
    case CALLSITE2:
      return CALLSITE2_STRING;
      break;
    case CALLSITE3:
      return CALLSITE3_STRING;
      break;
    case CALLSITE4:
      return CALLSITE4_STRING;
      break;
    case CALLSITE5:
      return CALLSITE5_STRING;
      break;
    case CALLSITE6:
      return CALLSITE6_STRING;
      break;
    case CALLSITE7:
      return CALLSITE7_STRING;
      break;
    case CALLSITE8:
      return CALLSITE8_STRING;
      break;
    case CALLSITE9:
      return CALLSITE9_STRING;
      break;

    case CALLER1:
      return CALLER1_STRING;
      break;
    case CALLER2:
      return CALLER2_STRING;
      break;
    case CALLER3:
      return CALLER3_STRING;
      break;
    case CALLER4:
      return CALLER4_STRING;
      break;
    case CALLER5:
      return CALLER5_STRING;
      break;
    case CALLER6:
      return CALLER6_STRING;
      break;
    case CALLER7:
      return CALLER7_STRING;
      break;
    case CALLER8:
      return CALLER8_STRING;
      break;
    case CALLER9:
      return CALLER9_STRING;
      break;

    case INSENSITIVE:
      return INSENSITIVE_STRING;
      break;
    default:
      assert(false);  // impossible
  }
}

auto operator>>(std::istream&, ContextSensitivity&) -> std::istream&;
