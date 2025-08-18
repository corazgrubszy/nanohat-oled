#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate src/yt_config.h with XOR-obfuscated YouTube API key and Channel ID.
Usage:
  ./tools/gen_yt_config.py --channel-id "YOUR_CHANNEL_ID" --api-key "YOUR_API_KEY" --out src/yt_config.h --key 0x5A
"""
import argparse
from pathlib import Path
from string import Template

HEADER_TEMPLATE = Template(r"""#pragma once

#include <stddef.h>
#include <stdint.h>

/* --- minimal helpers --- */
static inline void secure_wipe(void *p, size_t n) {
    volatile unsigned char *v = (volatile unsigned char*)p;
    while (n--) *v++ = 0;
}

static inline void xor_deobfuscate(const uint8_t *in, size_t len, uint8_t key, char *out) {
    for (size_t i = 0; i < len; ++i) out[i] = (char)(in[i] ^ key);
    out[len] = '\0';
}

/* build-time settings */
#define YT_XOR_K 0x${xor_key}

/* obfuscated payloads (XOR-ed with YT_XOR_K) */
static const uint8_t YT_KEY_OBF[] = {
${key_bytes}
};
static const size_t YT_KEY_LEN = ${key_len};

static const uint8_t YT_ID_OBF[] = {
${id_bytes}
};
static const size_t YT_ID_LEN = ${id_len};

/* public API */
static inline size_t get_yt_api_key(char *dst, size_t cap) {
    if (cap <= YT_KEY_LEN) return 0;
    xor_deobfuscate(YT_KEY_OBF, YT_KEY_LEN, YT_XOR_K, dst);
    return YT_KEY_LEN;
}

static inline size_t get_yt_channel_id(char *dst, size_t cap) {
    if (cap <= YT_ID_LEN) return 0;
    xor_deobfuscate(YT_ID_OBF, YT_ID_LEN, YT_XOR_K, dst);
    return YT_ID_LEN;
}
""")

def encode_xor_lines(data: bytes, key: int, cols: int = 12) -> str:
    # prepare elements with XOR and commas except for the last one
    items = [f"0x{(b ^ key) & 0xFF:02X}" for b in data]
    if items:
        items[:-1] = [s + "," for s in items[:-1]]
    # wrap lines every 'cols' elements
    out = []
    for i in range(0, len(items), cols):
        out.append("    " + " ".join(items[i:i+cols]))
    return "\n".join(out)

def main():
    ap = argparse.ArgumentParser(description="Generate XOR-obfuscated yt_config.h")
    ap.add_argument("--channel-id", required=True)
    ap.add_argument("--api-key", required=True)
    ap.add_argument("--out", default="src/yt_config.h")
    ap.add_argument("--key", default="0x5A", help="XOR byte (e.g., 0x5A or 90)")
    args = ap.parse_args()

    try:
        xor_key = int(args.key, 0) & 0xFF
    except Exception:
        raise SystemExit("Invalid --key (use a single byte, e.g., 0x5A).")

    ak = args.api_key.encode("utf-8")
    ch = args.channel_id.encode("utf-8")

    text = HEADER_TEMPLATE.substitute(
        xor_key = f"{xor_key:02X}",
        key_bytes = encode_xor_lines(ak, xor_key),
        key_len   = len(ak),
        id_bytes  = encode_xor_lines(ch, xor_key),
        id_len    = len(ch),
    )

    out_path = Path(args.out)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(text, encoding="utf-8")
    print(f"[ok] wrote {out_path} (key_len={len(ak)}, id_len={len(ch)}, xor=0x{xor_key:02X})")

if __name__ == "__main__":
    main()
