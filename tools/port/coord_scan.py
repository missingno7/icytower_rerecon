#!/usr/bin/env python3
"""List raw screen/frame coordinate occurrences in the historical sources.

Regenerates the raw occurrence list behind docs/port/COORDINATES.md.
Read-only: scans src/*.c and include/**/*.h (excluding include/allegro,
the pinned Allegro headers) and prints one tab-separated row per line hit:

    file<TAB>line<TAB>token<TAB>count<TAB>code

Numeric tokens are matched as whole integer literals (not part of a larger
number or identifier); float spellings such as 480.0f are also reported.
Comments are not stripped; classification is done by hand in the report.

Usage:
    python tools/port/coord_scan.py [--tokens 640,480,...] [--summary]
"""
import argparse
import os
import re
import sys
from collections import Counter

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

DEFAULT_TOKENS = [
    "640", "639", "480", "479", "320", "240",
    "SCREEN_W", "SCREEN_H", "screen", "swap_screen", "gfx_driver",
]


def token_regex(tok):
    if tok.isdigit():
        return re.compile(r"(?<![\w.])" + tok + r"(?![\w])(?:\.\d*f?)?")
    return re.compile(r"(?<![\w])" + re.escape(tok) + r"(?![\w])")


def source_files():
    src = os.path.join(ROOT, "src")
    for name in sorted(os.listdir(src)):
        if name.endswith(".c"):
            yield "src/" + name
    inc = os.path.join(ROOT, "include")
    for dirpath, dirnames, filenames in os.walk(inc):
        rel = os.path.relpath(dirpath, ROOT).replace(os.sep, "/")
        if rel.startswith("include/allegro"):
            dirnames[:] = []
            continue
        dirnames.sort()
        for name in sorted(filenames):
            if name.endswith(".h"):
                yield rel + "/" + name


def scan(tokens):
    pats = [(t, token_regex(t)) for t in tokens]
    for rel in source_files():
        with open(os.path.join(ROOT, rel), encoding="latin-1") as fh:
            for no, line in enumerate(fh, 1):
                for tok, pat in pats:
                    n = len(pat.findall(line))
                    if n:
                        yield rel, no, tok, n, line.strip()


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--tokens", help="comma-separated token list")
    ap.add_argument("--summary", action="store_true",
                    help="print per-file/per-token counts instead of rows")
    args = ap.parse_args()
    tokens = args.tokens.split(",") if args.tokens else DEFAULT_TOKENS
    rows = list(scan(tokens))
    if args.summary:
        c = Counter()
        for f, _, t, n, _ in rows:
            c[(f, t)] += n
        for (f, t), n in sorted(c.items()):
            print(f"{f}\t{t}\t{n}")
        print(f"TOTAL\t\t{sum(c.values())}")
        return 0
    for f, no, tok, n, code in rows:
        sys.stdout.write(f"{f}\t{no}\t{tok}\t{n}\t{code}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
