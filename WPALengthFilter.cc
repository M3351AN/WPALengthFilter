#if defined(__AVX2__) && defined(__x86_64__)
#include <immintrin.h>
#endif

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

static const int _fast_io = []() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  return 0;
}();

constexpr size_t kMinLineLength = 8;
constexpr size_t kMaxLineLength = 64;
constexpr size_t kReportInterval = 1000000;
constexpr size_t kProgressDivisor = 1000000;

#if defined(__AVX2__) && defined(__x86_64__)
constexpr size_t kAvxThreshold = 32;

inline bool IsAsciiAvx2(const char* str, size_t len) {
  size_t index = 0;
  while (index + kAvxThreshold <= len) {
    __m256i chunk =
        _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str + index));
    if (_mm256_movemask_epi8(chunk)) {
      return false;
    }
    index += kAvxThreshold;
  }

  for (; index < len; ++index) {
    if (static_cast<unsigned char>(str[index]) > 127) {
      return false;
    }
  }
  return true;
}
#endif

inline bool IsAsciiString(const char* str, size_t len) {
#if defined(__AVX2__) && defined(__x86_64__)
  if (len >= kAvxThreshold) {
    return IsAsciiAvx2(str, len);
  }
#endif
  for (size_t i = 0; i < len; ++i) {
    if (static_cast<unsigned char>(str[i]) > 127) {
      return false;
    }
  }
  return true;
}

int main(int argc, char* argv[]) {
  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
    return 1;
  }

  std::string input_filename = argv[1];
  std::string output_filename;

  if (argc == 2) {
    fs::path in(input_filename);
    output_filename = in.stem().string() + "_wpa" + in.extension().string();

  } else {
    output_filename = argv[2];
  }
  size_t total_count = 0;
  size_t kept_count = 0;
  size_t removed_count = 0;

  try {
    std::ifstream input_file(input_filename, std::ios::binary);
    if (!input_file) {
      throw std::runtime_error("Failed to open input file");
    }

    std::ofstream output_file(output_filename, std::ios::binary);
    if (!output_file) {
      throw std::runtime_error("Failed to create output file");
    }

    std::string line;
    size_t lines_processed = 0;

    while (std::getline(input_file, line)) {
      ++total_count;
      size_t line_length = line.length();

      if (line_length > 0 && line[line_length - 1] == '\r') {
        line.resize(--line_length);
      }

      if (line_length >= kMinLineLength && line_length <= kMaxLineLength) {
        if (IsAsciiString(line.data(), line_length)) {
          ++kept_count;
          output_file << line << '\n';
        } else {
          ++removed_count;
        }
      } else {
        ++removed_count;
      }

      if (++lines_processed % kReportInterval == 0) {
        std::printf("Processed: %zuM lines | Kept: %zu | Removed: %zu\r",
                    total_count / kProgressDivisor, kept_count, removed_count);
        std::fflush(stdout);
      }
    }

    input_file.close();
    output_file.close();

    const double keep_percentage =
        (total_count > 0) ? (100.0 * kept_count / total_count) : 0.0;

    std::printf(
        "\nProcessing complete! Total: %zu lines | Kept: %zu (%.2f%%)\n",
        total_count, kept_count, keep_percentage);
    std::printf("Results saved to: %s\n", output_filename.c_str());
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Error: %s\n", e.what());
    return 1;
  }
  return 0;
}