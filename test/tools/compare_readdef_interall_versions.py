#!/usr/bin/env python3
"""Compare baseline/candidate HPhi binaries on random InterAll inputs.

This tool is intended for behavior-equivalence checks during refactoring:
- generate valid random InterAll fixtures for FermionHubbard
- run both binaries with identical inputs
- compare key scalar outputs from output/zvo_energy.dat
"""

from __future__ import annotations

import argparse
import random
import re
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Dict, List, Tuple


ENERGY_KEYS = ("Energy", "Doublon", "Sz")


def run(cmd: List[str], cwd: Path, log_file: Path) -> None:
    with log_file.open("w", encoding="utf-8") as fh:
        proc = subprocess.run(cmd, cwd=str(cwd), stdout=fh, stderr=subprocess.STDOUT)
    if proc.returncode != 0:
        raise RuntimeError(f"command failed ({proc.returncode}): {' '.join(cmd)}")


def write_stan(path: Path) -> None:
    path.write_text(
        "\n".join(
            [
                'L = 4',
                'model = "FermionHubbard"',
                'method = "Lanczos"',
                'lattice = "chain"',
                't = 1.0',
                'U = 4.0',
                'nelec = 4',
                '2Sz = 0',
            ]
        )
        + "\n",
        encoding="utf-8",
    )


def make_interall_lines(rng: random.Random, nsite: int = 4) -> List[Tuple[List[int], float]]:
    n_pairs = rng.randint(1, 4)
    lines: List[Tuple[List[int], float]] = []
    for _ in range(n_pairs):
        a, b, c, d = rng.sample(range(nsite), 4)
        s1 = rng.randint(0, 1)
        s2 = rng.randint(0, 1)
        coeff = rng.uniform(-0.35, 0.35)
        if abs(coeff) < 0.05:
            coeff = 0.05 if coeff >= 0 else -0.05

        # Canonical form used in ArrangeInterAllOffDiagonal() checks.
        row = [a, s1, b, s1, c, s2, d, s2]
        pair = [d, s2, c, s2, b, s1, a, s1]
        lines.append((row, coeff))
        lines.append((pair, coeff))
    return lines


def write_interall(path: Path, rows: List[Tuple[List[int], float]]) -> None:
    header = [
        "========================",
        f"NInterAll {len(rows)}",
        "========================",
        "========zInterAll=======",
        "========================",
    ]
    body: List[str] = []
    for row, coeff in rows:
        body.append(
            f"{row[0]} {row[1]} {row[2]} {row[3]} "
            f"{row[4]} {row[5]} {row[6]} {row[7]} {coeff:.16f} 0.0"
        )
    path.write_text("\n".join(header + body) + "\n", encoding="utf-8")


def patch_modpara(src: Path, dst: Path) -> None:
    out: List[str] = []
    for line in src.read_text(encoding="utf-8").splitlines():
        if line.strip().startswith("initial_iv"):
            out.append("initial_iv     1")
        else:
            out.append(line)
    dst.write_text("\n".join(out) + "\n", encoding="utf-8")


def patch_namelist(src: Path, dst: Path, interall_name: str) -> None:
    out: List[str] = []
    for line in src.read_text(encoding="utf-8").splitlines():
        if re.match(r"^\s*ModPara\s+", line):
            out.append("ModPara modpara_fixed.def")
        else:
            out.append(line)
    out.append(f"InterAll {interall_name}")
    dst.write_text("\n".join(out) + "\n", encoding="utf-8")


def parse_energy(path: Path) -> Dict[str, float]:
    result: Dict[str, float] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        cols = line.split()
        if len(cols) >= 2 and cols[0] in ENERGY_KEYS:
            result[cols[0]] = float(cols[1])
    missing = [k for k in ENERGY_KEYS if k not in result]
    if missing:
        raise RuntimeError(f"missing keys in {path}: {missing}")
    return result


def run_one(binary: Path, trial_dir: Path, interall_src: Path) -> Dict[str, float]:
    trial_dir.mkdir(parents=True, exist_ok=True)
    write_stan(trial_dir / "stan.in")
    run([str(binary), "-sdry", "stan.in"], trial_dir, trial_dir / "sdry.log")

    patch_modpara(trial_dir / "modpara.def", trial_dir / "modpara_fixed.def")
    shutil.copy2(interall_src, trial_dir / "interall.def")
    patch_namelist(trial_dir / "namelist.def", trial_dir / "namelist_cmp.def", "interall.def")

    run([str(binary), "-e", "namelist_cmp.def"], trial_dir, trial_dir / "run.log")
    return parse_energy(trial_dir / "output" / "zvo_energy.dat")


def compare_dicts(a: Dict[str, float], b: Dict[str, float], tol: float) -> Dict[str, float]:
    diffs: Dict[str, float] = {}
    for key in ENERGY_KEYS:
        diffs[key] = abs(a[key] - b[key])
    if any(v > tol for v in diffs.values()):
        raise AssertionError(f"metric diff exceeded tol={tol}: {diffs}")
    return diffs


def main() -> int:
    p = argparse.ArgumentParser(description="compare baseline/candidate on random InterAll inputs")
    p.add_argument("--baseline", required=True, help="path to baseline HPhi binary")
    p.add_argument("--candidate", required=True, help="path to candidate HPhi binary")
    p.add_argument("--trials", type=int, default=20, help="number of random trials")
    p.add_argument("--seed", type=int, default=20260208, help="random seed")
    p.add_argument("--tol", type=float, default=1.0e-10, help="absolute tolerance for scalar metrics")
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
        root = Path(tempfile.mkdtemp(prefix="hphi_interall_compare_"))
        cleanup = True

    print(f"[compare] baseline={baseline}")
    print(f"[compare] candidate={candidate}")
    print(f"[compare] trials={args.trials} seed={args.seed} tol={args.tol}")
    print(f"[compare] work_dir={root}")

    success = False
    try:
        for i in range(args.trials):
            trial = root / f"trial_{i:03d}"
            trial.mkdir(parents=True, exist_ok=True)
            rows = make_interall_lines(rng)
            interall_src = trial / "interall_source.def"
            write_interall(interall_src, rows)

            a = run_one(baseline, trial / "baseline", interall_src)
            b = run_one(candidate, trial / "candidate", interall_src)
            diffs = compare_dicts(a, b, args.tol)
            print(
                f"[ok] trial={i:03d} "
                f"dE={diffs['Energy']:.3e} dD={diffs['Doublon']:.3e} dSz={diffs['Sz']:.3e}"
            )
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
