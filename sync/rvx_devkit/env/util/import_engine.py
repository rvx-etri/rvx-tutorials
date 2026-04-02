import os, sys
from pathlib import Path

engine_path = (Path(__file__).parent.parent / 'engine').resolve()

sys.path.append(str(engine_path))