# Stellar -  Nation Level Deep Packet Inspection Framework

Analyzed by gary senderson / xanthorox dev.**

---

For years, the Great Firewall (GFW) has been a symbol of censorship and control. The tools that power it have been a black box. Today, we shed some light on it.

I am releasing the source code for **Stellar**, a high-performance, modular deep packet inspection (DPI) framework believed to be a core component of the GFW's surveillance and analysis infrastructure.

This isn't just another networking tool. This is the real deal, an engine built for nation-state level traffic analysis. By releasing it, I hope to empower security researchers, network engineers, and the curious public. We can turn a tool of surveillance into a tool for transparency and defense.

## What is Stellar?

Stellar is a C-based framework for stateful, high-speed network traffic analysis. It's not a single application, but a powerful engine that can be configured to load various "decoder" modules, each specializing in a different protocol. It was designed for performance, scalability, and modularity.

### Core Features

- **High-Performance Packet Processing**: Built in C/C++ with a focus on speed. Uses multi-threading, CPU pinning, and efficient memory management to handle massive traffic loads.
- **Modular Architecture**: The core engine is protocol-agnostic. It loads protocol "decoders" (like `dns_decoder`, `http_decoder`, etc.) as shared libraries (`.bundle` files) at runtime. This allows it to be extended to analyze any protocol imaginable.
- **Stateful Analysis**: Stellar tracks network sessions (TCP and UDP), reassembles IP fragments and TCP streams, allowing decoders to see a clean, complete picture of the application-layer data.
- **Advanced Techniques**: Employs modern networking techniques like bloom filters for efficient tracking of duplicated packets and evicted sessions.
- **Configurable I/O**: Can read from live network interfaces (though this requires specific hardware/drivers like `marsio`) or, more conveniently for most, from standard PCAP files.

## The Decoders

The true power of Stellar lies in its modules. This leak includes the source code for several decoders, including:

- **`dns_decoder`**: A fully-featured DNS protocol analyzer.
- **`http_decoder`**: For inspecting HTTP traffic.
- **`ftp_decoder`**: For FTP.
- **`ssl_decoder`**: For analyzing SSL/TLS handshakes.
- And more...

These are not just simple parsers; they are integrated into the Stellar framework to perform deep analysis on the reassembled traffic streams.

## Building Stellar

I've preserved the original build structure. You'll need a standard Linux build environment.

#### Prerequisites

- A C/C++ compiler (e.g., `gcc`, `g++`)
- `cmake` (version 3.5 or higher)
- `make` or a similar build tool
- `git`

#### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone <repository_url>
    cd stellar
    ```

2.  **Navigate to the `stellar` project directory:**
    *Note: There are two 'stellar' directories. You want the inner one that contains the main `CMakeLists.txt`.*
    ```bash
    cd stellar
    ```

3.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

4.  **Run CMake to configure the project:**
    *CMake will automatically handle the bundled vendor libraries (like hyperscan, brotli, etc.).*
    ```bash
    cmake ..
    ```

5.  **Compile the code:**
    *This will build the main `stellar` executable, the core library, and all the decoders.*
    ```bash
    make -j$(nproc)
    ```

After a successful build, you will find the main executable at `infra/stellar` and the decoder bundles (e.g., `dns_decoder.bundle`) in their respective project folders.

## How to Use Stellar

Stellar is configured via a TOML file.

#### 1. Create a Configuration File

Create a file named `config.toml`. Here is a basic example to get you started, which reads from a PCAP file and uses the DNS decoder.

```toml
[instance]
id = 1

# Configure packet I/O to read from a pcap file
[packet_io]
mode = "pcapfile"
pcap_path = "/path/to/your/traffic.pcap" # <--- IMPORTANT: Change this path
pcap_done_exit = 1 # Exit when the pcap is finished
thread_num = 2
cpu_mask = [1, 2]

# Configure the session manager
[session_manager]
tcp_session_max = 50000
udp_session_max = 50000

# --- Module Configuration ---
# This section tells Stellar which decoders to load.
# The path should point to the compiled .bundle file.

[[module]]
name = "dns_decoder"
path = "/path/to/stellar/dns_decoder/dns_decoder.bundle" # <--- IMPORTANT: Change this path

# [[module]]
# name = "http_decoder"
# path = "/path/to/stellar/http_decoder/http_decoder.bundle"
```

**Important:** You must provide the **absolute paths** to your PCAP file and the `.bundle` files for the modules you want to load.

#### 2. Run Stellar

Execute the `stellar` binary from the `build` directory, pointing it to your configuration file with the `-c` flag.

```bash
# From within your 'build' directory
./infra/stellar -c /path/to/your/config.toml
```

Stellar will start, load the specified decoders, and begin processing the PCAP file. The output and logs from the decoders will be printed to your console or to the log file specified in the config.

## A Final Word

This is a complex tool. I have provided the source code and basic instructions. It's up to you, the community, to explore its full capabilities. Reverse engineer it, improve it, I don't Give a fk

-**xanthorox dev**
