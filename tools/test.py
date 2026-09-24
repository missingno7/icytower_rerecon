"""Acceptance suite; real corruption controls plus isolated transaction tests."""
import sys, unittest
from pathlib import Path
from common import ROOT
sys.path.insert(0,str(ROOT/'tools'))
if __name__=='__main__':
    suite=unittest.defaultTestLoader.discover(str(ROOT/'tests'),pattern='test_*.py')
    result=unittest.TextTestRunner(verbosity=1).run(suite)
    raise SystemExit(0 if result.wasSuccessful() else 1)
