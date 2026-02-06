# Changelog

All notable changes to hping3 are documented in this file.

## Modernization (2026)

Build system, portability, and code safety improvements to compile cleanly on
modern Linux (including ARM/aarch64), modern GCC, and current Tcl versions.

### Build system

- **Debian Trixie-only support** -- dropped non-Linux build paths and
  simplified `configure`/`Makefile.in` to match Debian defaults.
- **Multi-arch Tcl library detection** -- `configure` now searches
  architecture-specific library paths (`aarch64-linux-gnu`,
  `arm-linux-gnueabihf`, `x86_64-linux-gnu`, etc.) to find `libtcl`,
  and passes the correct `-L` flag to the linker.
- **Tcl 8.5/8.6 support** -- `configure` now searches for Tcl 8.5 and 8.6
  in addition to 8.0--8.4.
- **Fixed shell quoting bug** -- corrected unquoted `[ -n $USE_TCL ]` test
  in `configure` that could cause unexpected behavior.
- **Use PATH-based tools** -- `Makefile.in` now invokes `ar` and `ranlib`
  from `$PATH` instead of hardcoded `/usr/bin/` paths.
- **Removed hardcoded `-L/usr/local/lib`** from the linker command.
- **Added compiler flags** -- `-Wno-pointer-sign` (suppresses
  `char *`/`unsigned char *` sign warnings), `-D_DEFAULT_SOURCE` and
  `-D_GNU_SOURCE` (expose modern POSIX/GNU APIs and silence deprecation
  warnings for `_BSD_SOURCE`).

### Portability

- **Endianness bitfield macros** -- `configure` now emits
  `__LITTLE_ENDIAN_BITFIELD` / `__BIG_ENDIAN_BITFIELD` definitions in
  `byteorder.h` using GCC's `__BYTE_ORDER__` intrinsics, fixing builds on
  platforms (e.g., musl, some ARM toolchains) that don't define these in
  system headers.
- **`extern` for `delaytable`** -- changed the `delaytable` array in
  `hping2.h` from a definition to an `extern` declaration, fixing
  "multiple definition" linker errors with GCC 10+ (which defaults to
  `-fno-common`).

### Platform cleanup

- **Removed dead BSD PPP macros** -- deleted `PPPHDR_SIZE_FREEBSD`,
  `PPPHDR_SIZE_OPENBSD`, `PPPHDR_SIZE_NETBSD`, and `PPPHDR_SIZE_BSDI`
  from `hping2.h` (unused; only `PPPHDR_SIZE_LINUX` is used).
- **Removed dead SunOS `SA_INTERRUPT` block** -- `signal.c` no longer
  contains the `#ifdef SA_INTERRUPT` block (SunOS 4.x only); simplified
  to set `SA_RESTART` for non-SIGALRM signals only.
- **Removed dead `<net/bpf.h>` conditional** -- `libpcap_stuff.c` no
  longer contains the non-Linux `#include <net/bpf.h>` conditional
  (pcap.h provides everything needed on Debian).

### Code safety

- **Replaced `strcat` with `strlcpy`/`strlcat`** -- all flag-string
  construction in `main.c` and `waitpacket.c` now uses bounds-checked
  `strlcpy()`/`strlcat()` with explicit `sizeof` limits, eliminating
  potential buffer overflows.
- **Added `strlcat` implementation** -- `strlcpy.c` now provides an
  OpenBSD-compatible `strlcat()` for non-BSD platforms; declared in
  `hping2.h`.
- **Fixed type in `memstr.c`** -- changed `needlesize` from `char` to
  `size_t` to match the return type of `strlen()`, preventing truncation
  on long needle strings.

## Git History (2012--2014)

### 2014-12-26 -- Repository cleanup (antirez)

- Removed compilation warnings and applied minor fixes
- Removed object files and configure-generated files from the repository
- Added `.gitignore`
- Merged community contributions:
  - Fix checksum computation bug (Eric Wustrow)
  - macOS support fixes (clevertension)
  - Variable type correction (Gabriel Alves)

### 2014-06-26

- Fixed code to support macOS (clevertension)

### 2014-01-22

- Fixed incorrect checksum computation in `cksum.c` (Eric Wustrow)

### 2013-10-12

- Corrected variable type in data handling code (Gabriel Alves)
- Removed generated files (`Makefile`, `.depend`) from repository

### 2012-06-13

- Git repository created from `hping3-clockskew-0.tar.gz` (antirez)

## Pre-Git Releases

### hping3 alpha 2 (2004)

- Added `--beep` option: audible alert for every matching packet received
- Added `--flood` option: send packets as fast as possible without showing replies

### hping3 alpha 1 -- 2004-03-30

First public release of hping3. Major additions over hping2:

- Embedded Tcl scripting interpreter
- ARS (Archer Packet System) for packet description and compilation
- APD (Ars Packet Description) human-readable packet format
- `--scan` port scanning mode
- Standard library of Tcl scripts (`lib/*.htcl`)
- Interactive scripting console

### hping2

Previous major version. The hping2 command-line interface is retained in hping3 as a compatibility layer. Key capabilities carried forward:

- TCP, UDP, ICMP, and raw IP packet generation
- Packet fragmentation
- Source address spoofing
- Traceroute across all protocols
- TTL and IP option control
- TCP flag manipulation (SYN, ACK, FIN, RST, PUSH, URG)
- IP record route and source routing
- File-based payload injection
- Round-trip time measurement and statistics
