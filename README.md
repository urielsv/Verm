[![✗](https://img.shields.io/badge/Release-v1.1.0-ffb600.svg?style=for-the-badge)](https://github.com/agustin-golmar/Flex-Bison-Compiler/releases)

[![✗](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml/badge.svg?branch=production)](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml)

# Verm - Network Analysis Language

A domain-specific language for network traffic analysis, developed with Flex and Bison. Verm allows you to capture, filter, and analyze PCAP files with a simple, declarative syntax.

## Features

- **PCAP Processing**: Capture and analyze network traffic from PCAP files
- **Protocol Support**: Extract data from IP, TCP, UDP, HTTP protocols
- **Filtering**: Use BPF filters and custom conditions
- **Aggregation**: Group and count network events
- **HTML Dashboard**: Generate beautiful HTML reports with metrics and tables
- **Type Safety**: Semantic analysis with type checking

## Example Usage

```verm
capture from "traffic.pcap" where tcp.dst_port == 80;

extract {
    timestamp,
    ip.src_addr as client_ip,
    tcp.dst_port as port,
    http.response_code as status
};

filter status >= 400;

group by client_ip;

count errors;

export to "http_errors.html";
```

## Dependencies

### Required
- **GCC** or **Clang** (C compiler)
- **CMake** (3.22 or higher)
- **Flex** (lexical analyzer generator)
- **Bison** (parser generator)
- **libpcap** (network packet capture library)

### Installation

#### macOS (using Homebrew)
```bash
brew install cmake flex bison libpcap
```

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake flex bison libpcap-dev
```

#### Windows
- Install Visual Studio with C++ support
- Install CMake from https://cmake.org/download/
- Install Flex and Bison (available via MSYS2 or WSL)

## Building

### Quick Build
```bash
# Clone the repository
git clone <repository-url>
cd Verm

# Build the project
./script/ubuntu/build.sh
```

### Manual Build
```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
make
```

## Running

### Basic Usage
```bash
# Compile a Verm program
./Compiler input.verm

# The compiler will generate an executable that processes the PCAP file
# and produces an HTML dashboard report
```

### Test Cases
```bash
# Run acceptance tests
for file in src/test/c/accept/*.verm; do
    ./Compiler "$file"
done

# Run rejection tests (should fail)
for file in src/test/c/reject/*.verm; do
    ./Compiler "$file" 2>/dev/null || echo "Expected failure: $file"
done
```

## Project Structure

```
src/
├── main/c/
│   ├── frontend/           # Lexical and syntactic analysis
│   │   ├── lexical-analysis/
│   │   └── syntactic-analysis/
│   ├── backend/            # Semantic analysis and code generation
│   │   ├── semantic-analysis/
│   │   ├── code-generation/
│   │   └── runtime/
│   └── shared/             # Common utilities
├── test/c/                 # Test cases
│   ├── accept/             # Valid programs
│   └── reject/             # Invalid programs
└── script/                 # Build scripts
```

## Environment Variables

| Name | Default | Description |
|------|---------|-------------|
| `LOG_IGNORED_LEXEMES` | `true` | Log ignored lexemes at DEBUG level |
| `LOGGING_LEVEL` | `INFORMATION` | Minimum logging level (ALL, DEBUGGING, INFORMATION, WARNING, ERROR, CRITICAL) |

## Output Format

The compiler generates HTML dashboards with:
- **Metrics Cards**: Key statistics with color-coded indicators
- **Data Tables**: Detailed packet information
