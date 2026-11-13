"""Remove guest return-address authentication on Linux ARM64 hosts without PAC."""

import struct


PAC_TO_PLAIN = {
    0xD503233F: 0xD503201F,  # paciasp -> nop
    0xD503237F: 0xD503201F,  # pacibsp -> nop
    0xD50323BF: 0xD503201F,  # autiasp -> nop
    0xD50323FF: 0xD503201F,  # autibsp -> nop
    0xD65F0BFF: 0xD65F03C0,  # retaa -> ret
    0xD65F0FFF: 0xD65F03C0,  # retab -> ret
}


def strip_return_pac(image, code_ranges):
    counts = {word: 0 for word in PAC_TO_PLAIN}
    for start, end in code_ranges:
        for old, new in PAC_TO_PLAIN.items():
            pattern = struct.pack('<I', old)
            at = start
            while (at := image.find(pattern, at, end)) >= 0:
                if at % 4 == 0:
                    struct.pack_into('<I', image, at, new)
                    counts[old] += 1
                at += 1
    return counts


def strip_other_pac(image, code_ranges, disassembler):
    """Drop signing/authentication and use a repeatable PACGA checksum on hosts without PAC."""
    counts = {}
    for start, end in code_ranges:
        for at in range(start, end - 3, 4):
            if image[at + 3] not in (0x9A, 0xD5, 0xDA, 0xD6, 0xD7):
                continue
            ins = next(disassembler.disasm(bytes(image[at:at + 4]), at), None)
            if not ins:
                continue
            name = ins.mnemonic
            if name == 'pacga':
                old = struct.unpack_from('<I', image, at)[0]
                new = 0xCA000000 | (old & 0x001F0000) | (old & 0x3FF)
            elif name.startswith(('pac', 'aut', 'xpac')):
                new = 0xD503201F
            elif name.startswith(('braa', 'brab', 'blraa', 'blrab', 'retaa', 'retab', 'eretaa', 'eretab')):
                raise ValueError(f'authenticated branch at {at:#x}: {name}')
            else:
                continue
            struct.pack_into('<I', image, at, new)
            counts[name] = counts.get(name, 0) + 1
    return counts
