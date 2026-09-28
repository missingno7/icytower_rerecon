"""Inventory of the Allegro 4.4.1 API surface used by the 25 historical units (for the SDL3 port).
Writes docs/sdl3/allegro-api-usage.json: functions, variables, types, macros with use counts and a
per-file function map. Identifiers are matched against the pinned Allegro headers' AL_FUNC/AL_INLINE/
AL_VAR/AL_ARRAY/AL_FUNCPTR declarations, typedefs and #defines; comments and strings are stripped."""
import re, json, glob, os, sys
from collections import defaultdict, Counter
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
INC = ROOT / 'third_party/allegro-4.4.1/include'

def headers():
    text = ''
    for p in list(INC.rglob('*.h')) + list(INC.rglob('*.inl')):
        text += p.read_text(encoding='latin-1') + '\n'
    return text

def strip(text):
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    text = re.sub(r'"(?:[^"\\]|\\.)*"', '""', text)
    return text

def main():
    hdr = headers()
    funcs = set(re.findall(r'AL_FUNC\s*\(\s*[^,]+,\s*(\w+)\s*,', hdr)) | set(re.findall(r'AL_INLINE\s*\(\s*[^,]+,\s*(\w+)\s*,', hdr))
    variables = set(re.findall(r'AL_VAR\s*\(\s*[^,]+,\s*(\w+)\s*\)', hdr)) | set(re.findall(r'AL_ARRAY\s*\(\s*[^,]+,\s*(\w+)\s*\)', hdr)) | set(re.findall(r'AL_FUNCPTR\s*\(\s*[^,]+,\s*(\w+)\s*,', hdr))
    types = set(re.findall(r'typedef\s+struct\s+(\w+)', hdr)) | set(re.findall(r'\}\s*(\w+)\s*;', hdr)) | set(re.findall(r'typedef\s+[\w\s\*]+?\s(\w+)\s*;', hdr))
    macros = set(re.findall(r'^\s*#\s*define\s+(\w+)', hdr, re.M))
    prefixes = ('KEY_', 'GFX_', 'DIGI_', 'MIDI_', 'JOY', 'COLORCONV', 'SWITCH_', 'DRAW_MODE', 'MASK_COLOR', 'DAT_', 'END_OF_MAIN', 'ALLEGRO_',
                'ABS', 'MID', 'MAX', 'MIN', 'SGN', 'TRUE', 'FALSE', 'itofix', 'fixtoi', 'ftofix', 'fixtof', 'MAKE_VERSION', 'COLOR_DEPTH', 'SCREEN_W', 'SCREEN_H', 'DISPLAY_', 'LOAD_', 'SYSTEM_', 'D_', 'TIMERS_PER_SECOND', 'BPS_TO_TIMER', 'MSEC_TO_TIMER', 'SECS_TO_TIMER', 'GFX_AUTODETECT', 'AL_')
    macros = {m for m in macros if m.startswith(prefixes)}
    ids = Counter(); per_file = defaultdict(Counter)
    for p in sorted((ROOT / 'src').glob('*.c')):
        t = strip(p.read_text(encoding='cp1252'))
        for m in re.finditer(r'\b([A-Za-z_]\w*)\b', t):
            ids[m.group(1)] += 1; per_file[p.name][m.group(1)] += 1
    def used(names): return dict(sorted(((n, ids[n]) for n in names if ids.get(n)), key=lambda x: -x[1]))
    uf, uv, ut, um = used(funcs), used(variables), used(types), used(macros)
    inv = {'source': 'src/*.c (25 historical units) against the pinned Allegro 4.4.1 headers',
           'counts': {'functions': len(uf), 'variables': len(uv), 'types': len(ut), 'macros_and_constants': len(um)},
           'functions': uf, 'variables': uv, 'types': ut, 'macros_and_constants': um,
           'per_file_functions': {f: {n: c for n, c in cnt.items() if n in uf} for f, cnt in sorted(per_file.items())}}
    out = ROOT / 'docs/sdl3/allegro-api-usage.json'; out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(inv, indent=1), encoding='utf-8')
    print('functions %d | variables %d | types %d | macros %d' % (len(uf), len(uv), len(ut), len(um)))
    print('functions:', ', '.join('%s(%d)' % kv for kv in uf.items()))
    print('variables:', ', '.join('%s(%d)' % kv for kv in uv.items()))
    print('types:', ', '.join('%s(%d)' % kv for kv in ut.items()))
    print('macros:', ', '.join(um))

if __name__ == '__main__':
    main()
