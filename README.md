# WPALengthFilter
A tool that helps you filter the passwords in wordlists to only in legal WPA passwords format

## Features
- Remove passwords shorter than 8 characters
- Remove passwords longer than 64 characters
- Remove passwords containing non-ASCII characters

## Usage

```
./WPALengthFilter <input_file> <output_file>
```

## Build requirements
- A compiler that supports AVX2(Optional)
- A compiler that supports C++17 (GCC 7.1&later/Clang 5.0&later/MSVC v141&later)

## Runtime requirements
- A CPU that supports AVX2(Optional)
