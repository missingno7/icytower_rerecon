"""Decode with the identified objdump; never scan operand bytes as opcodes."""
import re
from common import run, sha


def decode(path, objdump, start=None, stop=None):
    args = [objdump, '-d', '-w']
    if start is not None:
        args += ['--start-address=%d' % start, '--stop-address=%d' % stop]
    args.append(path)
    rows = []
    section = None
    for line in run(args).splitlines():
        if line.startswith('Disassembly of section '):
            section = line.split('section ', 1)[1].rstrip(':')
        match = re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}\s)+)\s*(.*)', line)
        if match and section == '.text':
            raw = bytes.fromhex(match[2])
            asm = match[3].strip()
            rows.append({'address': int(match[1], 16), 'bytes': raw.hex(),
                         'mnemonic': asm.split()[0] if asm else '', 'assembly': asm})
    return rows

