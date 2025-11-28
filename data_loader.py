import os

import awkward as ak
import numpy as np
import pandas as pd
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


def load_and_process_data(root_file, cache=True):
    """
    加载 ROOT 文件并进行预处理
    返回: (data_ak, df_hits, process_map, num_events)
    """
    if not os.path.exists(root_file):
        raise FileNotFoundError(f"File '{root_file}' not found.")

    base_name = os.path.splitext(root_file)[0]
    map_file = f"{base_name}_ProcessIDMap.txt"
    process_map = load_process_map(map_file)

    # 缓存文件路径
    hits_cache = f"{base_name}_hits.parquet"
    primaries_cache = f"{base_name}_primaries.parquet"

    # 1. 加载 ROOT 数据 (Awkward Array) - 总是需要用于事件级统计
    print(f"Opening ROOT file: {root_file}...")
    file = uproot.open(root_file)
    if "CsI;1" not in file:
        raise ValueError(f"TTree 'CsI' not found in file.")

    tree = file["CsI"]
    # 读取所有分支
    branches = [
        "EventID",
        "TotalEdep",
        "HitCount",
        "CrystalID",
        "CrystalEdep",
        "CrystalTime",
        "CrystalPosX",
        "CrystalPosY",
        "CrystalPosZ",
        "CrystalPDG",
        "CrystalParentID",
        "CrystalDirX",
        "CrystalDirY",
        "CrystalDirZ",
        "CrystalKineticEnergy",
        "CrystalProcessID",
        # Primary Particle Branches
        "PrimaryPDG",
        "PrimaryEnergy",
        "PrimaryPosX",
        "PrimaryPosY",
        "PrimaryPosZ",
        "PrimaryDirX",
        "PrimaryDirY",
        "PrimaryDirZ",
    ]

    # 读取为 awkward array
    data_ak = tree.arrays(branches, library="ak")
    num_events = len(data_ak)
    print(f"Successfully loaded {num_events} events from ROOT.")

    # 2. 获取 DataFrame (优先读取缓存)
    df_hits = None
    df_primaries = None

    if cache and os.path.exists(hits_cache) and os.path.exists(primaries_cache):
        print(f"Loading DataFrames from cache...")
        try:
            df_hits = pd.read_parquet(hits_cache)
            df_primaries = pd.read_parquet(primaries_cache)
            print(f"Loaded {len(df_hits)} hits and {len(df_primaries)} primaries from cache.")
        except Exception as e:
            print(f"Failed to load cache: {e}. Will regenerate.")
            df_hits = None
            df_primaries = None

    if df_hits is None or df_primaries is None:
        print("Converting to Pandas DataFrames...")

        # --- 处理 Hits ---
        # 选取需要的列（包括新增的字段）
        arrays_to_flatten = data_ak[
            [
                "CrystalEdep",
                "CrystalTime",
                "CrystalPosX",
                "CrystalPosY",
                "CrystalPosZ",
                "CrystalPDG",
                "CrystalParentID",
                "CrystalDirX",
                "CrystalDirY",
                "CrystalDirZ",
                "CrystalKineticEnergy",
                "CrystalProcessID",
                "CrystalID",
            ]
        ]

        df_hits = ak.to_dataframe(arrays_to_flatten)

        # 重命名列
        df_hits.rename(
            columns={
                "CrystalEdep": "edep",
                "CrystalTime": "time",
                "CrystalPosX": "posX",
                "CrystalPosY": "posY",
                "CrystalPosZ": "posZ",
                "CrystalPDG": "pdg",
                "CrystalParentID": "parentID",
                "CrystalDirX": "dirX",
                "CrystalDirY": "dirY",
                "CrystalDirZ": "dirZ",
                "CrystalKineticEnergy": "kineticEnergy",
                "CrystalProcessID": "processID",
                "CrystalID": "crystalID",
            },
            inplace=True,
        )

        # 处理 EventID
        df_hits.reset_index(inplace=True)
        df_hits.rename(columns={"entry": "EventID", "subentry": "hit_idx"}, inplace=True)

        # 添加解码后的坐标
        ix, iy, iz = decode_crystal_id(df_hits["crystalID"].values)
        df_hits["ix"] = ix
        df_hits["iy"] = iy
        df_hits["iz"] = iz

        # --- 处理 Primaries ---
        print("Processing Primary particles...")
        primary_arrays = data_ak[["PrimaryPDG", "PrimaryEnergy", "PrimaryPosX", "PrimaryPosY", "PrimaryPosZ", "PrimaryDirX", "PrimaryDirY", "PrimaryDirZ"]]

        df_primaries = ak.to_dataframe(primary_arrays)
        df_primaries.rename(
            columns={
                "PrimaryPDG": "pdg",
                "PrimaryEnergy": "energy",
                "PrimaryPosX": "posX",
                "PrimaryPosY": "posY",
                "PrimaryPosZ": "posZ",
                "PrimaryDirX": "dirX",
                "PrimaryDirY": "dirY",
                "PrimaryDirZ": "dirZ",
            },
            inplace=True,
        )

        df_primaries.reset_index(inplace=True)
        df_primaries.rename(columns={"entry": "EventID", "subentry": "primary_idx"}, inplace=True)

        # 保存缓存
        if cache:
            print(f"Saving DataFrame caches...")
            df_hits.to_parquet(hits_cache)
            df_primaries.to_parquet(primaries_cache)

    return data_ak, df_hits, df_primaries, process_map, num_events
