# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

hping3 is a network tool for sending custom TCP/IP packets and displaying target replies. It supports TCP, UDP, ICMP, and raw IP protocols with packet fragmentation and arbitrary content. Version 3 adds Tcl scripting capabilities.

## Build Commands

```bash
# Debian Trixie
./configure          # Generates byteorder.h and Makefile
make                 # Build hping3
sudo make install    # Install to /usr/sbin/hping3
```

Build options:
- `./configure --no-tcl` - Build without Tcl scripting support
- `make strip` - Strip binary for smaller size
- `make clean` - Remove build artifacts
- `make distclean` - Full clean including generated files

## Dependencies

- **libpcap** (required) - Packet capture library
- **Tcl/Tk** (optional but recommended) - For scripting support
- **gcc** - Compiler
- Root access required for raw socket operations

## Architecture

### Dual-Layer Design

**Core Layer (hping2 compatibility):**
- Raw packet generation via `send*.c` files (sendtcp.c, sendudp.c, sendicmp.c, sendip.c, sendrawip.c)
- Packet reception via waitpacket.c and listen.c using libpcap
- Command-line interface in parseoptions.c and antigetopt.c

**Scripting Layer (hping3):**
- Tcl interpreter embedded in script.c (~37KB, the largest source file)
- ARS (Archer Packet System) for packet description/compilation in ars.c and apd.c
- Standard library in lib/hpingstdlib.htcl

### Key Components

| Component | Files | Purpose |
|-----------|-------|---------|
| Packet sending | `sendtcp.c`, `sendudp.c`, `sendicmp.c`, `sendip.c` | Protocol-specific packet construction |
| Packet reception | `waitpacket.c`, `listen.c`, `libpcap_stuff.c` | Receive/process packets via libpcap |
| ARS engine | `ars.c`, `apd.c`, `rapd.c`, `split.c` | APD packet description parser and compiler |
| Interface handling | `getifname.c`, `interface.c` | Network interface enumeration |
| Checksums | `cksum.c` | IP/TCP/UDP checksum computation |

### APD (Ars Packet Description) Format

Packets are represented as human-readable strings:
```
ip(saddr=192.168.1.6,daddr=213.82.224.41,ttl=64)+tcp(sport=123,dport=80,flags=s)
```

Key files for APD:
- `apd.c` - Parser/tokenizer
- `ars.c` - Compiler (APD to binary)
- `rapd.c` - Rapid packet assembly
- `apdutils.c` - Utility functions

### Global State

Main headers:
- `hping2.h` - Core type definitions and constants (TABLESIZE=400 for packet tracking)
- `globals.h` - Global variable declarations

## Running Tcl Scripts

```bash
# Interactive mode
hping3

# Execute a script
hping3 exec script.htcl [args]
```

Example scripts in `lib/`:
- `ping.htcl` - ICMP ping
- `ciscoios.htcl` - Cisco IOS fingerprinting
- `isn-spectrogram.htcl` - TCP ISN analysis

## Documentation

- `docs/API.txt` - Complete Tcl scripting API reference
- `docs/APD.txt` - APD packet description format
- `docs/HPING2-HOWTO.txt` - Usage guide
- `docs/hping3.8` - Man page
