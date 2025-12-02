import json
import os

import awkward as ak
import numpy as np
import pandas as pd
import uproot


def load_config(config_file="data_config.json"):
    """加载配置文件"""
    if not os.path.exists(config_file):
        raise FileNotFoundError(f"Config file '{config_file}' not found.")

    with open(config_file, "r", encoding="utf-8") as f:
        config = json.load(f)

    return config


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


def load_and_process_data(root_file, cache=True, config_file="data_config.json", return_awkward=True):
    """
    加载 ROOT 文件并进行预处理

    返回: (data_ak, df_hits, df_primaries, process_map, num_events)
    或者 (df_hits, df_primaries, process_map, num_events) 如果 return_awkward=False

    参数:
        root_file: ROOT文件路径
        cache: 是否使用缓存
        config_file: 配置文件路径
        return_awkward: 是否返回awkward数组（默认True）
    """
    if not os.path.exists(root_file):
        raise FileNotFoundError(f"File '{root_file}' not found.")

    # 加载配置
    config = load_config(config_file)

    # 如果配置中启用了 photon_settings，扩展 event_level 分支以包含 photon 相关向量
    photon_cfg = config.get("photon_settings", {})
    if photon_cfg.get("save_exit_counts", False):
        exit_names = photon_cfg.get("exit_branch_names", {})
        exit_ids_name = exit_names.get("ids", "PhotonExitCrystalIDs")
        exit_counts_name = exit_names.get("counts", "PhotonExitCounts")
        # 做一个本地的 event_level 分支列表副本并扩展
        event_level_branches = list(config["branches"]["event_level"]) + [exit_ids_name, exit_counts_name]
    else:
        event_level_branches = list(config["branches"]["event_level"])

    base_name = os.path.splitext(root_file)[0]
    map_file = base_name + config["cache_suffix"]["process_map"]
    process_map = load_process_map(map_file)

    # 缓存文件路径
    hits_cache = base_name + config["cache_suffix"]["hits"]
    primaries_cache = base_name + config["cache_suffix"]["primaries"]

    # 1. 加载 ROOT 数据 (Awkward Array) - 总是需要用于事件级统计
    print(f"Opening ROOT file: {root_file}...")
    file = uproot.open(root_file)
    tree_name = config["tree_name"] + ";1"
    if tree_name not in file:
        raise ValueError(f"TTree '{config['tree_name']}' not found in file.")

    tree = file[tree_name]

    # 从配置文件读取所有需要的分支
    branches = event_level_branches + config["branches"]["crystal_hits"] + config["branches"]["primary_particles"] + config["branches"]["photon_exit"]

    # 过滤掉不存在的分支
    available_branches = set(tree.keys())
    original_branches = branches[:]
    branches = [b for b in branches if b in available_branches]
    if len(branches) < len(original_branches):
        missing = set(original_branches) - set(branches)
        print(f"[Warning] Some branches not found in ROOT file: {missing}. Skipping them.")

    # 读取为 awkward array
    data_ak = tree.arrays(branches, library="ak")
    num_events = len(data_ak)
    print(f"Successfully loaded {num_events} events from ROOT.")

    # 2. 获取 DataFrame (优先读取缓存)
    df_hits = None
    df_primaries = None

    if cache and os.path.exists(hits_cache) and os.path.exists(primaries_cache):
        print("Loading DataFrames from cache...")
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
        # 从配置读取需要处理的Crystal分支
        crystal_branches = config["branches"]["crystal_hits"]
        arrays_to_flatten = data_ak[crystal_branches]

        df_hits = ak.to_dataframe(arrays_to_flatten)

        # 使用配置文件中的映射重命名列
        df_hits.rename(columns=config["column_mapping"]["hits"], inplace=True)

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
        primary_branches = config["branches"]["primary_particles"]
        primary_arrays = data_ak[primary_branches]

        df_primaries = ak.to_dataframe(primary_arrays)

        # 使用配置文件中的映射重命名列
        df_primaries.rename(columns=config["column_mapping"]["primaries"], inplace=True)

        df_primaries.reset_index(inplace=True)
        df_primaries.rename(columns={"entry": "EventID", "subentry": "primary_idx"}, inplace=True)

        # 保存缓存
        if cache:
            print("Saving DataFrame caches...")
            df_hits.to_parquet(hits_cache)
            df_primaries.to_parquet(primaries_cache)

    # 返回结果
    if return_awkward:
        return data_ak, df_hits, df_primaries, process_map, num_events
    else:
        return df_hits, df_primaries, process_map, num_events


def load_awkward_only(root_file, config_file="data_config.json"):
    """
    只加载并返回awkward数组，不进行DataFrame转换
    适合只需要事件级别分析或原始数据结构的场景

    返回: (data_ak, process_map, num_events)
    """
    if not os.path.exists(root_file):
        raise FileNotFoundError(f"File '{root_file}' not found.")

    # 加载配置
    config = load_config(config_file)

    base_name = os.path.splitext(root_file)[0]
    map_file = base_name + config["cache_suffix"]["process_map"]
    process_map = load_process_map(map_file)

    # 加载 ROOT 数据
    print(f"Opening ROOT file: {root_file}...")
    file = uproot.open(root_file)
    tree_name = config["tree_name"] + ";1"
    if tree_name not in file:
        raise ValueError(f"TTree '{config['tree_name']}' not found in file.")

    tree = file[tree_name]

    # 从配置文件读取所有需要的分支
    photon_cfg = config.get("photon_settings", {})
    if photon_cfg.get("save_exit_counts", False):
        exit_names = photon_cfg.get("exit_branch_names", {})
        exit_ids_name = exit_names.get("ids", "PhotonExitCrystalIDs")
        exit_counts_name = exit_names.get("counts", "PhotonExitCounts")
        event_branches = list(config["branches"]["event_level"]) + [exit_ids_name, exit_counts_name]
    else:
        event_branches = list(config["branches"]["event_level"])

    branches = event_branches + config["branches"]["crystal_hits"] + config["branches"]["primary_particles"]

    # 过滤掉不存在的分支
    available_branches = set(tree.keys())
    original_branches = branches[:]
    branches = [b for b in branches if b in available_branches]
    if len(branches) < len(original_branches):
        missing = set(original_branches) - set(branches)
        print(f"[Warning] Some branches not found in ROOT file: {missing}. Skipping them.")

    # 读取为 awkward array
    data_ak = tree.arrays(branches, library="ak")
    num_events = len(data_ak)
    print(f"Successfully loaded {num_events} events (awkward array only).")

    return data_ak, process_map, num_events


def get_awkward_arrays(data_ak, config_file="data_config.json"):
    """
    从完整的awkward数组中提取不同部分的子数组

    参数:
        data_ak: 完整的awkward数组
        config_file: 配置文件路径

    返回: (event_data, hits_data, primary_data)
        - event_data: 事件级别数据 (EventID, TotalEdep, HitCount等)
        - hits_data: Crystal hit数据
        - primary_data: Primary粒子数据
    """
    config = load_config(config_file)

    event_branches = config["branches"]["event_level"]
    hit_branches = config["branches"]["crystal_hits"]
    primary_branches = config["branches"]["primary_particles"]

    # 检查并过滤不存在的分支
    available_branches = set(data_ak.fields)
    event_branches = [b for b in event_branches if b in available_branches]
    hit_branches = [b for b in hit_branches if b in available_branches]
    primary_branches = [b for b in primary_branches if b in available_branches]

    event_data = data_ak[event_branches]
    hits_data = data_ak[hit_branches]
    primary_data = data_ak[primary_branches]

    return event_data, hits_data, primary_data
