#include <boost/filesystem.hpp>
#include <exception>
#include <sstream>
#include <string>

class ParseException : public std::exception {
  std::string err_msg;

 public:
  ParseException(const boost::filesystem::path& inputFile) {
    std::ostringstream msg;

    // Construct error message
    msg << "Failed to parse " << inputFile;
    err_msg = msg.str();
  }

  ~ParseException() noexcept override = default;

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return err_msg.c_str();
  }
};
