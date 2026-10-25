#!/usr/bin/env python3
"""Compare CSV output from two search benchmark runs."""

import argparse
import csv


def read_results(path):
    results = {}
    with open(path, newline="", encoding="utf-8") as source:
        for row in csv.DictReader(source):
            key = (row["scenario"], int(row["depth"]))
            if key in results:
                raise ValueError(f"Duplicate benchmark case in {path}: {key}")
            results[key] = row
    if not results:
        raise ValueError(f"No benchmark results in {path}")
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("before", help="CSV from the baseline version")
    parser.add_argument("after", help="CSV from the changed version")
    args = parser.parse_args()

    before = read_results(args.before)
    after = read_results(args.after)
    if before.keys() != after.keys():
        raise ValueError("Benchmark cases differ between the two files")

    print("scenario,depth,before_ms,after_ms,speedup,move_changed")
    for scenario, depth in sorted(before):
        old = before[scenario, depth]
        new = after[scenario, depth]
        if old["iterations"] != new["iterations"]:
            raise ValueError(f"Iteration count differs for {scenario} at depth {depth}")
        old_ms = float(old["median_ms"])
        new_ms = float(new["median_ms"])
        if new_ms <= 0:
            raise ValueError(f"Invalid duration for {scenario} at depth {depth}")
        fields = ("move_from", "move_to", "promotion")
        move_changed = any(old[field] != new[field] for field in fields)
        print(
            f"{scenario},{depth},{old_ms:.3f},{new_ms:.3f},"
            f"{old_ms / new_ms:.2f}x,{str(move_changed).lower()}"
        )


if __name__ == "__main__":
    main()
