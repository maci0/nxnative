import struct
import unittest

from capstone import Cs, CS_ARCH_ARM64, CS_MODE_ARM
from dm.tools.nso_pac import PAC_TO_PLAIN, strip_other_pac, strip_return_pac


class ReturnPacTest(unittest.TestCase):
    def test_rewrites_only_aligned_code_words(self):
        words = list(PAC_TO_PLAIN)
        data = bytearray(b''.join(struct.pack('<I', w) for w in words) + b'\0' +
                         struct.pack('<I', words[-1]) + b'\0\0\0' + struct.pack('<I', words[0]))
        counts = strip_return_pac(data, [(0, 4 * len(words)), (4 * len(words) + 1, len(data))])
        self.assertEqual(sum(counts.values()), len(words) + 1)
        self.assertEqual([struct.unpack_from('<I', data, 4 * i)[0] for i in range(len(words))],
                         list(PAC_TO_PLAIN.values()))
        self.assertEqual(data[4 * len(words) + 1:4 * len(words) + 5], struct.pack('<I', words[-1]))

    def test_other_pac_and_context_checksum(self):
        words = [0xDAC107FE, 0xDAC117FE, 0xDAC143FA, 0x9AD13262]
        data = bytearray(b''.join(struct.pack('<I', w) for w in words))
        counts = strip_other_pac(data, [(0, len(data))], Cs(CS_ARCH_ARM64, CS_MODE_ARM))
        self.assertEqual(counts, {'pacib': 1, 'autib': 1, 'xpaci': 1, 'pacga': 1})
        self.assertEqual([w[0] for w in struct.iter_unpack('<I', data)],
                         [0xD503201F, 0xD503201F, 0xD503201F, 0xCA110262])


if __name__ == '__main__':
    unittest.main()
