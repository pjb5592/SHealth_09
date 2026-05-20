#!/usr/bin/env python3
"""Parse gcov .gcov files for shealth_lib and write coverage_summary.txt."""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

SHEALTH_SOURCES = (
    "SHealth.cpp",
    "CsvLoader.cpp",
    "Imputation.cpp",
    "Statistics.cpp",
    "SHealthBmiReport.cpp",
    "BmiDomain.h",
)


def run_gcov(object_dir: Path, source: str) -> None:
    gcno = object_dir / f"{source}.gcno"
    if not gcno.is_file():
        return
    subprocess.run(
        ["gcov", "-b", str(gcno)],
        cwd=object_dir,
        check=False,
        capture_output=True,
        text=True,
    )


def parse_gcov(gcov_path: Path) -> tuple[float, int, int, float, int, int]:
    """Return (line_pct, covered, exec_lines, branch_pct, taken, total_branches)."""
    covered = exec_lines = 0
    taken = total_br = 0
    if not gcov_path.is_file():
        return 0.0, 0, 0, 0.0, 0, 0

    line_re = re.compile(r"^\s*(\d+|#####):")
    branch_re = re.compile(r"branch\s+\d+\s+taken\s+(\d+)%")

    with gcov_path.open(encoding="utf-8", errors="replace") as handle:
        for line in handle:
            match = line_re.match(line)
            if match:
                exec_lines += 1
                if match.group(1) != "#####":
                    covered += 1
            bmatch = branch_re.search(line)
            if bmatch:
                # MinGW gcov marks STL exception edges as "(throw)"; not product branches.
                if "(throw)" in line:
                    continue
                total_br += 1
                if int(bmatch.group(1)) > 0:
                    taken += 1

    line_pct = 100.0 * covered / exec_lines if exec_lines else 0.0
    branch_pct = 100.0 * taken / total_br if total_br else 0.0
    return line_pct, covered, exec_lines, branch_pct, taken, total_br


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--object-dir", required=True, type=Path)
    parser.add_argument("--summary-out", required=True, type=Path)
    args = parser.parse_args()

    object_dir = args.object_dir.resolve()
    rows: list[dict[str, str | float | int]] = []

    for source in SHEALTH_SOURCES:
        run_gcov(object_dir, source)
        line_pct, covered, exec_lines, branch_pct, taken, total_br = parse_gcov(
            object_dir / f"{source}.gcov"
        )
        rows.append(
            {
                "file": source,
                "line_pct": line_pct,
                "lines": f"{covered}/{exec_lines}",
                "branch_pct": branch_pct,
                "branches": f"{taken}/{total_br}",
                "covered": covered,
                "exec_lines": exec_lines,
                "taken": taken,
                "total_br": total_br,
            }
        )

    cpp_rows = [r for r in rows if str(r["file"]).endswith(".cpp")]
    total_covered = sum(int(r["covered"]) for r in cpp_rows)
    total_exec = sum(int(r["exec_lines"]) for r in cpp_rows)
    total_taken = sum(int(r["taken"]) for r in cpp_rows)
    total_br = sum(int(r["total_br"]) for r in cpp_rows)

    agg_line = 100.0 * total_covered / total_exec if total_exec else 0.0
    agg_branch = 100.0 * total_taken / total_br if total_br else 0.0

    lines_out: list[str] = [
        "SHealth shealth_lib coverage (gcov; branch % excludes STL exception probe edges)",
        "",
        f"{'File':<22} {'Line %':>8} {'Lines':>10} {'Branch %':>10} {'Branches':>10}",
    ]
    for row in rows:
        lines_out.append(
            f"{row['file']:<22} {row['line_pct']:7.1f}% {row['lines']:>10} "
            f"{row['branch_pct']:9.1f}% {row['branches']:>10}"
        )
    lines_out.extend(
        [
            "",
            f"shealth_lib .cpp aggregate: {agg_line:.1f}% lines ({total_covered}/{total_exec}), "
            f"{agg_branch:.1f}% branches ({total_taken}/{total_br})",
            "",
            "Goals (test_plan.md section 8.1): lines >= 90%, branches >= 85%",
            f"Line goal:    {'PASS' if agg_line >= 90.0 else 'FAIL'}",
            f"Branch goal:  {'PASS' if agg_branch >= 85.0 else 'FAIL'}",
        ]
    )

    text = "\n".join(lines_out) + "\n"
    args.summary_out.write_text(text, encoding="utf-8")
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
