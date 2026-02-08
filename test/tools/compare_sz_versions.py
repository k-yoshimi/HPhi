#!/usr/bin/env python3
"""Compare baseline/candidate HPhi binaries for sz-related outputs.

The script generates random valid standard-mode inputs and compares:
- output/CHECK_Sdim.dat (parsed sdim)
- output/CHECK_Memory.dat (parsed idim_max)
between baseline and candidate binaries.
"""

from __future__ import annotations

import argparse
import random
import re
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Dict, List


def run(cmd: List[str], cwd: Path, log_file: Path) -> None:
    with log_file.open("w", encoding="utf-8") as fh:
        proc = subprocess.run(cmd, cwd=str(cwd), stdout=fh, stderr=subprocess.STDOUT)
    if proc.returncode != 0:
        raise RuntimeError(f"command failed ({proc.returncode}): {' '.join(cmd)}")


def build_random_stan(rng: random.Random) -> str:
    model = rng.choice(["FermionHubbard", "Spin"])
    lines: List[str] = []

    if model == "FermionHubbard":
        lsize = rng.randint(2, 5)
        nelec = rng.randint(1, 2 * lsize - 1)
        lines.extend(
            [
                f"L = {lsize}",
                'model = "FermionHubbard"',
                'method = "FullDiag"',
                'lattice = "chain"',
                't = 1.0',
                'U = 4.0',
                f"nelec = {nelec}",
            ]
        )
        if rng.choice([True, False]):
            two_sz_candidates: List[int] = []
            for two_sz in range(-nelec, nelec + 1, 2):
                nup = (nelec + two_sz) // 2
                ndown = (nelec - two_sz) // 2
                if 0 <= nup <= lsize and 0 <= ndown <= lsize:
                    two_sz_candidates.append(two_sz)
            if two_sz_candidates:
                lines.append(f"2Sz = {rng.choice(two_sz_candidates)}")
    else:
        lsize = rng.randint(2, 8)
        two_sz_candidates = list(range(-lsize, lsize + 1, 2))
        lines.extend(
            [
                f"L = {lsize}",
                'model = "Spin"',
                'method = "FullDiag"',
                'lattice = "chain"',
                'J = 1.0',
                f"2Sz = {rng.choice(two_sz_candidates)}",
            ]
        )

    return "\n".join(lines) + "\n"


def parse_sdim(path: Path) -> int:
    text = path.read_text(encoding="utf-8")
    for line in text.splitlines():
        m = re.search(r"sdim=([0-9]+)", line)
        if m:
            return int(m.group(1))

    # fallback for table-style lines: e.g. "3 64"
    for line in text.splitlines():
        cols = line.strip().split()
        if len(cols) == 2 and cols[0].isdigit() and cols[1].isdigit():
            return int(cols[1])

    raise RuntimeError(f"failed to parse sdim from {path}")


def parse_idim(path: Path) -> int:
    text = path.read_text(encoding="utf-8")
    m = re.search(r"idim_max=([0-9]+)", text)
    if not m:
        raise RuntimeError(f"failed to parse idim_max from {path}")
    return int(m.group(1))


def run_one(binary: Path, case_dir: Path, stan_text: str) -> Dict[str, int]:
    case_dir.mkdir(parents=True, exist_ok=True)
    (case_dir / "stan.in").write_text(stan_text, encoding="utf-8")
    run([str(binary), "-s", "stan.in"], case_dir, case_dir / "run.log")
    sdim = parse_sdim(case_dir / "output" / "CHECK_Sdim.dat")
    idim = parse_idim(case_dir / "output" / "CHECK_Memory.dat")
    return {"sdim": sdim, "idim_max": idim}


def main() -> int:
    p = argparse.ArgumentParser(description="compare baseline/candidate sz outputs")
    p.add_argument("--baseline", required=True, help="path to baseline HPhi binary")
    p.add_argument("--candidate", required=True, help="path to candidate HPhi binary")
    p.add_argument("--trials", type=int, default=30, help="number of random trials")
    p.add_argument("--seed", type=int, default=20260208, help="random seed")
    p.add_argument(
        "--work-dir",
        default="",
        help="directory to keep trial artifacts; default is temporary and auto-clean",
    )
    args = p.parse_args()

    baseline = Path(args.baseline).resolve()
    candidate = Path(args.candidate).resolve()
    if not baseline.is_file():
        raise FileNotFoundError(f"baseline binary not found: {baseline}")
    if not candidate.is_file():
        raise FileNotFoundError(f"candidate binary not found: {candidate}")

    rng = random.Random(args.seed)
    if args.work_dir:
        root = Path(args.work_dir).resolve()
        root.mkdir(parents=True, exist_ok=True)
        cleanup = False
    else:
        root = Path(tempfile.mkdtemp(prefix="hphi_sz_compare_"))
        cleanup = True

    print(f"[compare] baseline={baseline}")
    print(f"[compare] candidate={candidate}")
    print(f"[compare] trials={args.trials} seed={args.seed}")
    print(f"[compare] work_dir={root}")

    success = False
    try:
        for i in range(args.trials):
            trial = root / f"trial_{i:03d}"
            stan_text = build_random_stan(rng)
            a = run_one(baseline, trial / "baseline", stan_text)
            b = run_one(candidate, trial / "candidate", stan_text)
            if a != b:
                raise AssertionError(f"trial={i:03d} mismatch baseline={a} candidate={b}")
            print(f"[ok] trial={i:03d} sdim={a['sdim']} idim_max={a['idim_max']}")
        success = True
    except Exception as exc:
        print(f"[fail] {exc}")
        print(f"[fail] retained artifacts at: {root}")
        return 1
    finally:
        if cleanup and success:
            shutil.rmtree(root, ignore_errors=True)

    print("[pass] all trials matched")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
