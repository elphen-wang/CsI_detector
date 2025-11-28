import argparse
import os
import sys

import awkward as ak
import matplotlib.pyplot as plt
import numpy as np
import uproot


def load_process_map(map_file):
    """读取 ProcessIDMap.txt 文件"""
    process_map = {}
    if not os.path.exists(map_file):
        print(f"[Warning] Process map file '{map_file}' not found. Process IDs will not be decoded.")
        return process_map

    try:
        with open(map_file, "r") as f:
            lines = f.readlines()
            for line in lines[1:]:  # Skip header
                parts = line.strip().split()
                if len(parts) >= 2:
                    pid = int(parts[0])
                    name = parts[1]
                    process_map[pid] = name
        print(f"[Info] Loaded {len(process_map)} processes from '{map_file}'")
    except Exception as e:
        print(f"[Error] Failed to read process map: {e}")

    return process_map


def decode_crystal_id(crystal_ids):
    """
    解码 CrystalID (XXYYZZ)
    返回 x, y, z 索引数组
    """
    ix = crystal_ids // 10000
    iy = (crystal_ids % 10000) // 100
    iz = crystal_ids % 100
    return ix, iy, iz


def analyze(root_file, output_dir="plots"):
    if not os.path.exists(root_file):
        print(f"Error: File '{root_file}' not found.")
        return

    # 尝试自动寻找对应的 ProcessIDMap 文件
    # 规则: data/xxx.root -> data/xxx_ProcessIDMap.txt
    base_name = os.path.splitext(root_file)[0]
    map_file = f"{base_name}_ProcessIDMap.txt"
    process_map = load_process_map(map_file)

    print(f"Opening ROOT file: {root_file}...")
    try:
        with uproot.open(root_file) as file:
            if "CsI;1" in file:
                tree = file["CsI"]
            else:
                print(f"Error: TTree 'CsI' not found in file. Available keys: {file.keys()}")
                return

            # 读取数据 (使用 awkward array 处理 vector)
            print("Reading data...")
            branches = ["TotalEdep", "HitCount", "CrystalEdep", "CrystalTime", "CrystalPosX", "CrystalPosY", "CrystalPDG", "CrystalProcessID", "CrystalID"]
            data = tree.arrays(branches, library="ak")

            num_events = len(data)
            print(f"Total Events: {num_events}")

            # 创建输出目录
            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            # ==========================================
            # 1. 总能量沉积谱 (Total Energy Deposition)
            # ==========================================
            plt.figure(figsize=(10, 6))
            plt.hist(data["TotalEdep"], bins=100, range=(0, max(data["TotalEdep"]) * 1.1), histtype="stepfilled", alpha=0.7, label="Total Edep")
            plt.xlabel("Energy (MeV)")
            plt.ylabel("Counts")
            plt.title(f"Total Energy Deposition per Event ({num_events} events)")
            plt.grid(True, alpha=0.3)
            plt.savefig(f"{output_dir}/TotalEdep.png")
            print(f"Saved {output_dir}/TotalEdep.png")

            # ==========================================
            # 2. 击中数分布 (Hit Multiplicity)
            # ==========================================
            plt.figure(figsize=(10, 6))
            plt.hist(data["HitCount"], bins=range(0, 20), align="left", rwidth=0.8, color="orange", alpha=0.7)
            plt.xlabel("Number of Hits")
            plt.ylabel("Counts")
            plt.title("Hit Multiplicity per Event")
            plt.xticks(range(0, 20))
            plt.grid(True, axis="y", alpha=0.3)
            plt.savefig(f"{output_dir}/HitCount.png")
            print(f"Saved {output_dir}/HitCount.png")

            # --- 展平数据以分析单个 Hit ---
            flat_edep = ak.flatten(data["CrystalEdep"])
            flat_time = ak.flatten(data["CrystalTime"])
            flat_posX = ak.flatten(data["CrystalPosX"])
            flat_posY = ak.flatten(data["CrystalPosY"])
            flat_pdg = ak.flatten(data["CrystalPDG"])
            flat_process = ak.flatten(data["CrystalProcessID"])
            flat_id = ak.flatten(data["CrystalID"])

            # ==========================================
            # 3. 击中位置分布 (Hit Positions X-Y)
            # ==========================================
            plt.figure(figsize=(8, 8))
            plt.hist2d(flat_posX, flat_posY, bins=50, cmap="viridis", cmin=1)
            plt.colorbar(label="Counts")
            plt.xlabel("X Position (mm)")
            plt.ylabel("Y Position (mm)")
            plt.title("Hit Positions (X-Y)")
            plt.axis("equal")
            plt.savefig(f"{output_dir}/HitPositions_XY.png")
            print(f"Saved {output_dir}/HitPositions_XY.png")

            # ==========================================
            # 4. 粒子类型分布 (PDG ID)
            # ==========================================
            unique_pdgs, counts = np.unique(flat_pdg, return_counts=True)
            # 常用 PDG 映射
            pdg_names = {22: "Gamma", 11: "e-", -11: "e+", 13: "mu-", -13: "mu+", 2112: "Neutron", 2212: "Proton", 1000020040: "Alpha"}
            labels = [f"{pdg_names.get(pid, str(pid))}\n({pid})" for pid in unique_pdgs]

            plt.figure(figsize=(10, 6))
            plt.bar(range(len(unique_pdgs)), counts, tick_label=labels, color="green", alpha=0.6)
            plt.xlabel("Particle Type")
            plt.ylabel("Counts")
            plt.title("Particle Types in Hits")
            plt.yscale("log")  # 对数坐标
            plt.grid(True, axis="y", alpha=0.3)
            plt.savefig(f"{output_dir}/ParticleTypes.png")
            print(f"Saved {output_dir}/ParticleTypes.png")

            # ==========================================
            # 5. 物理过程分布 (Process ID)
            # ==========================================
            unique_procs, proc_counts = np.unique(flat_process, return_counts=True)
            proc_labels = [process_map.get(pid, f"ID {pid}") for pid in unique_procs]

            # 排序
            sorted_indices = np.argsort(proc_counts)[::-1]
            sorted_counts = proc_counts[sorted_indices]
            sorted_labels = np.array(proc_labels)[sorted_indices]

            # 只显示前 10 个
            top_n = 15
            if len(sorted_counts) > top_n:
                sorted_counts = sorted_counts[:top_n]
                sorted_labels = sorted_labels[:top_n]

            plt.figure(figsize=(12, 6))
            plt.barh(range(len(sorted_counts)), sorted_counts, color="purple", alpha=0.6)
            plt.yticks(range(len(sorted_counts)), sorted_labels)
            plt.xlabel("Counts")
            plt.title(f"Top {top_n} Creator Processes")
            plt.gca().invert_yaxis()  # 让最多的在上面
            plt.xscale("log")
            plt.tight_layout()
            plt.savefig(f"{output_dir}/Processes.png")
            print(f"Saved {output_dir}/Processes.png")

            # ==========================================
            # 6. 时间分布 (Time)
            # ==========================================
            plt.figure(figsize=(10, 6))
            plt.hist(flat_time, bins=100, range=(0, 100), color="red", alpha=0.6)  # 假设 0-100ns
            plt.xlabel("Time (ns)")
            plt.ylabel("Counts")
            plt.title("Hit Time Distribution (0-100 ns)")
            plt.yscale("log")
            plt.grid(True, alpha=0.3)
            plt.savefig(f"{output_dir}/TimeDistribution.png")
            print(f"Saved {output_dir}/TimeDistribution.png")

    except Exception as e:
        print(f"An error occurred during analysis: {e}")
        import traceback

        traceback.print_exc()


def main():
    parser = argparse.ArgumentParser(description="Analyze Geant4 ROOT output.")
    parser.add_argument("file", help="Path to the ROOT file")
    parser.add_argument("-o", "--output", default="plots", help="Output directory for plots")

    args = parser.parse_args()

    analyze(args.file, args.output)


if __name__ == "__main__":
    main()
