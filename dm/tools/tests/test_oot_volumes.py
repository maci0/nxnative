"""The optional v131072 volume probe patches allocations and their matching dispatches."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'dm/tools'))
path = ROOT / 'dm/games/0400486024DEC000/patches.py'
spec = importlib.util.spec_from_file_location('oot_patches', path)
patches = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patches)


class Context:
    def __init__(self):
        self.checked = {}
        self.written = {}

    def site(self, address, original):
        assert address not in self.checked
        self.checked[address] = original

    def put(self, address, words, reason):
        assert address in self.checked and address not in self.written
        self.written[address] = words[0]


class OotVolumePatchTest(unittest.TestCase):
    def test_allocations_and_dispatches_match(self):
        ctx = Context()
        patches.half_volumes(ctx)
        self.assertEqual(len(ctx.written), 24)
        for start, count, old_x, old_y, x, y in ((0xb44e5c, 6, '021080d2', '0209c0f2', 0xd2800802, 0xf2c00482),
                                                 (0xb44f04, 4, '020880d2', '8204c0f2', 0xd2800402, 0xf2c00242)):
            for site in range(start, start + count * 0x1c, 0x1c):
                self.assertEqual(ctx.checked[site], old_x)
                self.assertEqual(ctx.checked[site + 0x14], old_y)
                self.assertEqual(ctx.written[site], x)
                self.assertEqual(ctx.written[site + 0x14], y)
        self.assertEqual({site: ctx.checked[site] for site in (0x292708, 0x292724, 0x293eec, 0x293f08)},
                         {0x292708: '2101080b', 0x292724: '62010a0b',
                          0x293eec: '2101080b', 0x293f08: '62010a0b'})
        self.assertEqual({site: ctx.written[site] for site in (0x292708, 0x292724, 0x293eec, 0x293f08)},
                         {0x292708: 0x52800081, 0x292724: 0x528000a2,
                          0x293eec: 0x52800101, 0x293f08: 0x528000a2})


if __name__ == '__main__':
    unittest.main()
