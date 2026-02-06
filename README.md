# hping3

hping3 is a network tool for sending custom TCP/IP packets and displaying target replies, similar to how `ping` works with ICMP replies. It supports TCP, UDP, ICMP, and RAW-IP protocols, packet fragmentation, arbitrary packet size and content, and traceroute-like functionality across all supported protocols.

Version 3 adds Tcl scripting capabilities and the ARS (Archer Packet System) for programmatic packet construction and analysis.

## Features

- **Custom packet crafting** -- send TCP, UDP, ICMP, and raw IP packets with full control over headers and content
- **Port scanning** -- built-in scan mode (`--scan`) with service name resolution
- **Traceroute** -- protocol-independent traceroute across TCP, UDP, and ICMP
- **OS fingerprinting** -- remote OS detection through TCP/IP stack behavior
- **Firewall testing** -- test firewall rules and ACLs with crafted packets
- **Path MTU discovery** -- determine maximum transmission unit along a path
- **TCP/IP stack auditing** -- analyze and test network stack implementations
- **Clock skew detection** -- detect remote host clock drift via TCP timestamps
- **Tcl scripting** -- embedded Tcl interpreter for writing custom network tools
- **APD packet description** -- human-readable packet format for scripting (`ip(saddr=1.2.3.4)+tcp(dport=80,flags=s)`)

## Quick Start

```bash
# Build
./configure
make

# Install (requires root)
sudo make install

# Send TCP SYN packets to port 80
sudo hping3 -S -p 80 example.com

# ICMP ping
sudo hping3 -1 example.com

# UDP mode
sudo hping3 -2 -p 53 example.com

# Port scan
sudo hping3 --scan 1-1024 -S example.com

# Traceroute via TCP
sudo hping3 -S -p 80 -T example.com
```

## Building from Source

### Requirements (Debian Trixie)

- **libpcap** (required) -- packet capture library
- **Tcl/Tk** (optional, recommended) -- for scripting support (8.5+ detected automatically)
- **gcc** -- C compiler
- Root access is required at runtime for raw socket operations

Install dependencies:

```bash
sudo apt-get update
sudo apt-get install -y build-essential libpcap-dev tcl-dev
```

### Debian Trixie Build

```bash
./configure
make
sudo make install    # installs to /usr/sbin/hping3
```

### Build Options

| Command | Description |
|---------|-------------|
| `./configure --no-tcl` | Build without Tcl scripting support |
| `make strip` | Strip the binary for smaller size |
| `make clean` | Remove build artifacts |
| `make distclean` | Full clean including generated files |

## Tcl Scripting

hping3 embeds a Tcl interpreter for writing custom network tools and scripts.

```bash
# Interactive mode
sudo hping3

# Run a script
sudo hping3 exec lib/ping.htcl <target>
```

Example scripts in `lib/`:

| Script | Purpose |
|--------|---------|
| `ping.htcl` | ICMP ping |
| `ciscoios.htcl` | Cisco IOS fingerprinting |
| `isn-spectrogram.htcl` | TCP ISN analysis |
| `nat-noise.htcl` | NAT noise analysis |
| `show-tcpseq.htcl` | TCP sequence number display |
| `passivets.htcl` | Passive timestamp analysis |

Packets can be described using the APD (Ars Packet Description) format:

```
ip(saddr=192.168.1.6,daddr=10.0.0.1,ttl=64)+tcp(sport=12345,dport=80,flags=s)
```

See `docs/API.txt` for the full scripting API and `docs/APD.txt` for the packet description format.

## Command-Line Usage

```
usage: hping host [options]
```

### Modes

| Flag | Mode |
|------|------|
| (default) | TCP |
| `-0` / `--rawip` | Raw IP |
| `-1` / `--icmp` | ICMP |
| `-2` / `--udp` | UDP |
| `-8` / `--scan` | Port scan |
| `-9` / `--listen` | Listen |

### Common Options

| Flag | Description |
|------|-------------|
| `-c` / `--count` | Number of packets to send |
| `-i` / `--interval` | Interval between packets (e.g., `-i u1000` for 1000 microseconds) |
| `--fast` | Send 10 packets per second |
| `--faster` | Send 100 packets per second |
| `--flood` | Send as fast as possible |
| `-a` / `--spoof` | Spoof source address |
| `-t` / `--ttl` | Set TTL (default 64) |
| `-S` / `--syn` | Set SYN flag |
| `-A` / `--ack` | Set ACK flag |
| `-F` / `--fin` | Set FIN flag |
| `-p` / `--destport` | Destination port |
| `-s` / `--baseport` | Source port |
| `-T` / `--traceroute` | Traceroute mode |
| `-d` / `--data` | Payload data size |
| `-E` / `--file` | Read payload from file |

Run `hping3 -h` for the full list of options.

## Documentation

- `docs/API.txt` -- Tcl scripting API reference
- `docs/APD.txt` -- APD packet description format
- `docs/HPING2-HOWTO.txt` -- usage guide
- `docs/hping3.8` -- man page (`man hping3` after install)

## Recent Modernization

The codebase has been updated to build cleanly with modern toolchains:

- **Modern GCC support** -- fixes for `-fno-common` (default since GCC 10), suppression of pointer-sign warnings, and proper POSIX/GNU feature macros
- **Multi-arch Linux** -- configure detects Tcl libraries on aarch64, armhf, x86_64, and other architectures
- **Tcl 8.5/8.6** -- version detection updated from the original 8.0--8.4 range
- **Buffer safety** -- replaced `strcat` with bounds-checked `strlcpy`/`strlcat` throughout
- **Portability** -- endianness bitfield macros generated for toolchains that lack them
- **Platform cleanup** -- removed dead BSD (FreeBSD, OpenBSD, NetBSD, BSDI) and SunOS platform code; codebase now targets Debian/Linux only
- **Performance** -- scan mode uses compact active-port list and batched sends; cached `signlen` in listen mode; static packet buffer in `wait_packet()`; pre-parsed `--rand-dest` template

See [CHANGELOG.md](CHANGELOG.md) for full details.

## License

GNU General Public License version 2. See [COPYING](COPYING) for the full license text.

## Author

Created and maintained by Salvatore Sanfilippo ([@antirez](https://github.com/antirez)).

See [AUTHORS](AUTHORS) for the full list of contributors.
