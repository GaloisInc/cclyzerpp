#include "ContextSensitivity.hpp"

auto operator>>(std::istream& in, ContextSensitivity& sensitivity)
    -> std::istream& {
  std::string token;
  in >> token;

  if (token == INSENSITIVE_STD_STRING) {
    sensitivity = INSENSITIVE;

  } else if (token == CALLSITE1_STD_STRING) {
    sensitivity = CALLSITE1;
  } else if (token == CALLSITE2_STD_STRING) {
    sensitivity = CALLSITE2;
  } else if (token == CALLSITE3_STD_STRING) {
    sensitivity = CALLSITE3;
  } else if (token == CALLSITE4_STD_STRING) {
    sensitivity = CALLSITE4;
  } else if (token == CALLSITE5_STD_STRING) {
    sensitivity = CALLSITE5;
  } else if (token == CALLSITE6_STD_STRING) {
    sensitivity = CALLSITE6;
  } else if (token == CALLSITE7_STD_STRING) {
    sensitivity = CALLSITE7;
  } else if (token == CALLSITE8_STD_STRING) {
    sensitivity = CALLSITE8;
  } else if (token == CALLSITE9_STD_STRING) {
    sensitivity = CALLSITE9;

  } else if (token == CALLER1_STD_STRING) {
    sensitivity = CALLER1;
  } else if (token == CALLER2_STD_STRING) {
    sensitivity = CALLER2;
  } else if (token == CALLER3_STD_STRING) {
    sensitivity = CALLER3;
  } else if (token == CALLER4_STD_STRING) {
    sensitivity = CALLER4;
  } else if (token == CALLER5_STD_STRING) {
    sensitivity = CALLER5;
  } else if (token == CALLER6_STD_STRING) {
    sensitivity = CALLER6;
  } else if (token == CALLER7_STD_STRING) {
    sensitivity = CALLER7;
  } else if (token == CALLER8_STD_STRING) {
    sensitivity = CALLER8;
  } else if (token == CALLER9_STD_STRING) {
    sensitivity = CALLER9;

  } else {
    in.setstate(std::ios_base::failbit);
  }

  return in;
}
