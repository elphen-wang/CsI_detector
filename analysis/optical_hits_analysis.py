#!/usr/bin/env python3
"""
optical_hits_analysis.py

读取 Geant4 输出 ROOT 文件（默认：build/CsI_Axion.root），统计每个 CsI 晶体的出射光子次数。
输出：
 - build/optical_hits_summary.csv : 每个晶体的总出射光子数
 - build/optical_hits_per_event.npy : 每事件出射光子总数数组（numpy）
 - build/optical_hits_top.png : 前 20 个晶体的柱状图

依赖： uproot, awkward, numpy, pandas, matplotlib
安装： pip install uproot awkward numpy pandas matplotlib

用法：
    python3 analysis/optical_hits_analysis.py [ROOT_FILE]

如果没有提供 ROOT 文件，默认使用 build/CsI_Axion.root
"""

import os
import sys
from collections import defaultdict

try:
    import awkward as ak
    import matplotlib.pyplot as plt
    import numpy as np
    import pandas as pd
    import uproot
except Exception as e:
    print("Missing dependency:", e)
    print("Please install requirements: pip install uproot awkward numpy pandas matplotlib")
    sys.exit(1)


def analyze(root_path: str = "build/CsI_Axion.root"):
    if not os.path.exists(root_path):
        print(f"ROOT file not found: {root_path}")
        return 1

    print(f"Opening: {root_path}")
    f = uproot.open(root_path)
    # Try common tree name(s)
    tree_name = None
    for candidate in ["CsI", "CsI_Axion", "tree"]:
        if candidate in f:
            tree_name = candidate
            break
    if tree_name is None:
        # pick the first TTree-like object
        for k in f.keys():
            try:
                obj = f[k]
                # object that has keys() considered tree-like
                _ = obj.keys()
                tree_name = k
                break
            except Exception:
                continue

    if tree_name is None:
        print("No tree found in ROOT file.")
        return 1

    tree = f[tree_name]
    print("Found tree:", tree_name)
    branches = list(tree.keys())
    print("Branches:", branches)

    if "PhotonExitCrystalID" not in branches or "PhotonExitCount" not in branches:
        print("PhotonExit branches not found. Available branches:", branches)
        return 1

    # Read branches as awkward arrays
    arr = tree.arrays(["PhotonExitCrystalID", "PhotonExitCount"], library="ak")
    ids = arr["PhotonExitCrystalID"]
    counts = arr["PhotonExitCount"]

    nevents = len(ids)
    print(f"Events: {nevents}")

    total_per_crystal = defaultdict(int)
    per_event_total = np.zeros(nevents, dtype=np.int64)

    # Convert to python lists lazily to avoid memory surprises
    for i in range(nevents):
        id_list = ak.to_list(ids[i]) if ids[i] is not None else []
        cnt_list = ak.to_list(counts[i]) if counts[i] is not None else []
        if len(id_list) != len(cnt_list):
            # Defensive: sometimes one of the arrays may be empty or jagged differently
            # Try to pair as much as possible
            m = min(len(id_list), len(cnt_list))
            id_list = id_list[:m]
            cnt_list = cnt_list[:m]
        event_sum = 0
        for cid, cc in zip(id_list, cnt_list):
            try:
                cid_int = int(cid)
                cnt_int = int(cc)
            except Exception:
                continue
            total_per_crystal[cid_int] += cnt_int
            event_sum += cnt_int
        per_event_total[i] = event_sum

    # Summary DataFrame
    if len(total_per_crystal) == 0:
        print("No optical exit counts found in file (all zero).")
    else:
        df = pd.DataFrame(list(total_per_crystal.items()), columns=["CrystalID", "TotalExitCount"])
        df = df.sort_values(by="TotalExitCount", ascending=False).reset_index(drop=True)
        out_csv = os.path.join("build", "optical_hits_summary.csv")
        df.to_csv(out_csv, index=False)
        print(f"Saved per-crystal summary to: {out_csv}")

        # Save top 20 plot
        topn = 20
        fig, ax = plt.subplots(figsize=(10, 6))
        df_top = df.head(topn)
        ax.bar(df_top["CrystalID"].astype(str), df_top["TotalExitCount"])
        ax.set_xlabel("CrystalID")
        ax.set_ylabel("Total exiting optical photons")
        ax.set_title(f"Top {topn} crystals by exiting optical photons")
        plt.xticks(rotation=45)
        plt.tight_layout()
        out_png = os.path.join("build", "optical_hits_top.png")
        fig.savefig(out_png)
        print(f"Saved top-{topn} bar plot: {out_png}")

    # Save per-event totals
    np.save(os.path.join("build", "optical_hits_per_event.npy"), per_event_total)
    print("Saved per-event totals (numpy) to build/optical_hits_per_event.npy")

    # Print quick stats
    print("Total events:", nevents)
    print("Events with non-zero exits:", np.count_nonzero(per_event_total))
    print("Total exiting photons:", per_event_total.sum())

    print("Done.")
    return 0


if __name__ == "__main__":
    root_path = sys.argv[1] if len(sys.argv) > 1 else "build/CsI_Axion.root"
    sys.exit(analyze(root_path))
