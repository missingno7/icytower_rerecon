#!/usr/bin/env python3
"""Synthetic Icy Tower replays (.itr, format ITR140) for differential tests.

Writes replays whose input streams are generated from a seed, in the exact
format of replay.c save_replay() with a valid checksum, so both the frozen
reference build and the portable build accept them with `-check`.  The
anti-cheat timing tables are left zero, which keeps the checksum free of
floating point.

    python tools/port/mkreplay.py OUTDIR --count 200 [--seed 1] [--custom]
        [--style mixed|random|climber|runner]

Styles
  random   uniformly random key states and hold times
  climber  mostly jumps with short sideways runs (climbs, makes combos)
  runner   long runs into the walls followed by jumps (wall bounces, fast
           rotating jumps, large combos)
  mixed    a different style per replay
With --custom the game settings (floor size, speed, gravity, rejump) are
randomised too, exercising the custom-game branches.
"""
import argparse, random, struct
from pathlib import Path

LEFT, RIGHT, FIRE = 1, 2, 16


def hash32(a):
    a &= 0xFFFFFFFF
    a = (a ^ 0x3D) ^ (a >> 16)
    a = (a * 9) & 0xFFFFFFFF
    a ^= a >> 4
    a = (a * 668265261) & 0xFFFFFFFF
    a ^= a >> 15
    return a & 0xFFFFFFFF


def s8(b):
    return b - 256 if b >= 128 else b


def checksum(r):
    """calc_replay_checksum() of replay.c (tc tables zero)."""
    M = 0xFFFFFFFF
    s = (r['biggest_lost_combo'] * 17 + 17 + r['no_combo_top_floor'] * 127) & M
    s = (s + s + r['floor_shrink'] * 102 + r['floor_size'] * 17 + 3702 + r['start_speed'] * 163 +
         r['speed_increase'] * 23 + r['gravity'] * 88) & M
    s = (s + r['score'] * 17 + 17 + r['tc_posts'] * 127 + r['random_seed'] * 329 + (r['combo'] + 1) * 73 +
         r['rejump'] * 13 + (r['floor'] + 1) * 113) & M
    for i in range(5):
        s = (s + r['ccc'][i] * (39 + i * 3) + r['jc'][i] * (27 + i * 3)) & M
    date, name, comment = r['date'], r['name'], r['comment']
    for i in range(32):
        s = (s + (s8(date[i]) + i) * (s8(name[i]) + i) * (17 + i * 17)) & M
    for i in range(42):
        c = s8(comment[i])
        s = (s + (c + i) * (c + i) * (-3 + i * 3)) & M
    for i, (flags, count) in enumerate(r['data']):
        s = (s + count * 7 * (i % 167 + 1) + flags * 3 * (i % 193 + 1)) & M
    h = hash32(s)
    return h - (1 << 32) if h >= (1 << 31) else h


def pad(s, n):
    b = s.encode('latin-1')[:n - 1]
    return b + b'\0' * (n - len(b))


def write_replay(path, r):
    r = dict(r)
    r['checksum'] = checksum(r)
    out = bytearray()
    out += b'ITR140'
    out += struct.pack('<i', len(r['data']))
    out += r['name'] + r['date']
    for k in ('score', 'floor', 'combo', 'no_combo_top_floor', 'biggest_lost_combo'):
        out += struct.pack('<i', r[k])
    out += struct.pack('<5i', *r['ccc']) + struct.pack('<5i', *r['jc'])
    for k in ('floor_shrink', 'floor_size', 'start_speed', 'speed_increase', 'gravity', 'rejump', 'random_seed'):
        out += struct.pack('<i', r[k])
    out += r['comment']
    out += struct.pack('<i', r['checksum'])
    out += struct.pack('<i', r['tc_posts'])
    out += b'\0' * (100 * 5 * 4)
    for flags, count in r['data']:
        out += struct.pack('<iB', count, flags)
    Path(path).write_bytes(bytes(out))


def gen_inputs(rng, style, n):
    data = []
    for _ in range(n):
        st = style
        if st == 'random':
            flags = rng.choice([0, LEFT, RIGHT, FIRE, LEFT | FIRE, RIGHT | FIRE])
            count = rng.randint(0, 40)
        elif st == 'climber':
            d = rng.choice([LEFT, RIGHT])
            flags = rng.choice([FIRE, d | FIRE, d, d | FIRE, 0])
            count = rng.randint(0, 18)
        else:  # runner
            d = rng.choice([LEFT, RIGHT])
            flags = rng.choice([d, d, d | FIRE, FIRE, 0])
            count = rng.randint(4, 60) if flags == d else rng.randint(0, 12)
        data.append((flags, count))
    return data


def make(rng, idx, style, custom):
    if style == 'mixed':
        style = rng.choice(['random', 'climber', 'runner'])
    r = {
        'name': pad(f'synth{idx}', 32), 'date': pad('28 Sep 2026', 32),
        'score': 0, 'floor': 0, 'combo': 0, 'no_combo_top_floor': 0, 'biggest_lost_combo': 0,
        'ccc': [0] * 5, 'jc': [0] * 5,
        'floor_shrink': 1, 'floor_size': 1, 'start_speed': 5, 'speed_increase': 1, 'gravity': 1,
        'rejump': rng.choice([0, 1]), 'random_seed': rng.randint(0, 0x7FFF),
        'comment': pad(style, 42), 'tc_posts': 0,
    }
    if custom:
        r.update(floor_size=rng.randint(0, 4), start_speed=rng.randint(0, 5), gravity=rng.randint(0, 2),
                 floor_shrink=rng.choice([0, 1]), speed_increase=rng.choice([0, 1]))
    r['data'] = gen_inputs(rng, style, rng.randint(300, 4000))
    return r


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('outdir')
    ap.add_argument('--count', type=int, default=100)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--style', default='mixed', choices=['mixed', 'random', 'climber', 'runner'])
    ap.add_argument('--custom', action='store_true')
    a = ap.parse_args()
    out = Path(a.outdir)
    out.mkdir(parents=True, exist_ok=True)
    rng = random.Random(a.seed)
    for i in range(a.count):
        write_replay(out / f'synth_{a.seed}_{i:04d}.itr', make(rng, i, a.style, a.custom))
    print(f'wrote {a.count} replays to {out}')


if __name__ == '__main__':
    main()
