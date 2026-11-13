# SLSsteam Linux i386 build and recovery

## Architecture

The upstream v2.9 release `bin/SLSsteam.so` was inspected: ELF32, little-endian, Intel 80386/i386. The replacement was built with the project's `-m32` configuration and the Actions job verified the output with `file` and `readelf`.

## Build and test record

GitHub Actions run: [Build Linux i386 SLSsteam #3](https://github.com/laphp1k-hash/slsteam-moon/actions/runs/43337970372). All steps passed on commit `92ae952f4003cfcabf77edab848473019cb2bcc1`:

- i386 compiler and 32-bit OpenSSL/curl dependency checks
- SHA-1 known-answer test (`abc`)
- simulated missing `SHA1` symbol: returned an explicit error and left the caller buffer unchanged
- full project build
- 32-bit `dlopen` of `SLSsteam.so` and `la_version` audit ABI smoke test
- ELF architecture and linked dependency checks
- artifact upload

The supplied binary's checksum matches the checksum generated and uploaded by Actions. A live test inside the user's Steam/Bazzite process and a byte-for-byte digest comparison against the affected user's AppInfo cache file were not possible because that file and Steam environment were not available here. The SHA-1 implementation was verified with the known-answer vector and the failure path test.

## Install

1. Exit Steam completely. Do not change any LuaTools, Lumen, or game data.
2. Back up the existing hook and note its hash:

   ```bash
   cd "$HOME/.local/share/SLSsteam"
   cp -a SLSsteam.so "SLSsteam.so.backup-$(date +%Y%m%d-%H%M%S)"
   sha256sum SLSsteam.so
   ```

3. Copy `SLSsteam.so` and `SLSsteam.so.sha256` from the downloaded outputs to `~/.local/share/SLSsteam/`. Update another library copy only if your Lumen/Millennium configuration explicitly references that path.
4. Verify architecture and checksum:

   ```bash
   file "$HOME/.local/share/SLSsteam/SLSsteam.so"
   readelf -h "$HOME/.local/share/SLSsteam/SLSsteam.so" | grep -E 'Class:|Machine:'
   sha256sum -c SLSsteam.so.sha256
   ```

   Expected: ELF 32-bit, Intel 80386; SHA-256 `794795e04490b744e87a3c7f760da4622460a983ff24bb387abb4e00e6a80230`.
5. Start Steam and check the SLSsteam log for successful loading and AppInfo validation.

## Rollback

Exit Steam and restore the matching backup:

```bash
cp -a "$HOME/.local/share/SLSsteam/SLSsteam.so.backup-YYYYMMDD-HHMMSS" \
  "$HOME/.local/share/SLSsteam/SLSsteam.so"
```

## Safe AppInfo cache regeneration

Generated cache pairs live in SLSsteam's configured `cache/` directory and are named `picsbuffer_<appid>.bin` and `picsbuffer_<appid>.yaml`. Do not remove the entire cache or Steam library. First back up the cache (adjust the path if your configuration differs):

```bash
stamp=$(date +%Y%m%d-%H%M%S)
backup="$HOME/.local/share/SLSsteam/cache-backup-$stamp"
mkdir -p "$backup"
cp -a "$HOME/.local/share/SLSsteam/cache/." "$backup/"
```

After installing the fix, let LuaTools/SLSsteam refresh naturally. If an app remains stale, exit Steam and move only that app's matching `.bin` and `.yaml` pair into the backup; restart Steam/LuaTools and trigger that app's AppInfo refresh. Restore the pair if regeneration fails. Preserve `dlcmetadata_*.yaml`, game scripts, LuaTools configuration, and all other files.

