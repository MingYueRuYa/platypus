#ifndef string_utils_
#define string_utils_

#include <codecvt>  // codecvt_utf8
#include <locale>   // wstring_convert
// encoding function
std::string to_utf8_string(const std::wstring& wide_string) {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
  return utf8_conv.to_bytes(wide_string);
}

inline std::wstring to_wide_string(const std::string& input) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.from_bytes(input);
}

#endif  // string_utils_