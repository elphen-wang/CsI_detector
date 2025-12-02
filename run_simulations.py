import argparse
import multiprocessing
import os
import shutil
import subprocess
import sys
import time

# ================= Default Configuration =================
DEFAULT_CONFIG = {"EXECUTABLE_NAME": "CsI_Axion", "BUILD_DIR": "build", "DATA_DIR": "data", "NUM_JOBS": 50, "EVENTS_PER_JOB": 10000, "OUTPUT_PREFIX": "output_"}
# =========================================================


class SimConfig:
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)


def parse_arguments():
    parser = argparse.ArgumentParser(description="Run Geant4 simulations in parallel.")
    parser.add_argument("output", nargs="?", default="result.root", help="Output ROOT filename (default: result.root)")
    parser.add_argument("-j", "--jobs", type=int, default=DEFAULT_CONFIG["NUM_JOBS"], help=f"Number of parallel jobs (default: {DEFAULT_CONFIG['NUM_JOBS']})")
    parser.add_argument("-n", "--events", type=int, default=DEFAULT_CONFIG["EVENTS_PER_JOB"], help=f"Events per job (default: {DEFAULT_CONFIG['EVENTS_PER_JOB']})")

    args = parser.parse_args()

    output_filename = args.output
    if not output_filename.endswith(".root"):
        output_filename += ".root"

    return SimConfig(
        executable_name=DEFAULT_CONFIG["EXECUTABLE_NAME"],
        build_dir=DEFAULT_CONFIG["BUILD_DIR"],
        data_dir=DEFAULT_CONFIG["DATA_DIR"],
        num_jobs=args.jobs,
        events_per_job=args.events,
        output_prefix=DEFAULT_CONFIG["OUTPUT_PREFIX"],
        output_filename=output_filename,
    )


def check_environment(config):
    # Check build dir
    if not os.path.exists(config.build_dir):
        print(f"Error: Build directory '{config.build_dir}' does not exist.")
        return False

    # Check executable
    exe_path = os.path.join(config.build_dir, config.executable_name)
    if not os.path.exists(exe_path):
        print(f"Error: Executable '{config.executable_name}' not found in '{config.build_dir}'. Please build first.")
        return False

    # Prepare data dir
    if not os.path.exists(config.data_dir):
        os.makedirs(config.data_dir)
        print(f"Created directory: {config.data_dir}")

    # Check output file overwrite
    target_path = os.path.join(config.data_dir, config.output_filename)
    if os.path.exists(target_path):
        print(f"\nWARNING: The target file '{target_path}' already exists.")
        response = input("Do you want to overwrite it? (y/N): ").strip().lower()
        if response != "y":
            print("Aborted by user.")
            return False
        print("File will be overwritten.")

    return True


def run_single_job(args):
    """
    Worker function.
    args is a tuple (job_id, config) because pool.map takes one argument.
    """
    job_id, config = args

    start_time = time.time()
    work_dir = os.path.join(config.build_dir, f"work_{job_id}")

    # Cleanup and create work dir
    if os.path.exists(work_dir):
        shutil.rmtree(work_dir)
    os.makedirs(work_dir)

    try:
        # Copy executable
        exe_src = os.path.join(config.build_dir, config.executable_name)
        exe_dst = os.path.join(work_dir, config.executable_name)
        shutil.copy(exe_src, exe_dst)

        # Generate macro
        mac_content = f"""
/run/initialize
/CsI/generator/mode ePairDeflected
/CsI/random/seed {job_id} {job_id + 12345}
/CsI/random/apply  1
/run/beamOn {config.events_per_job}
"""
        mac_path = os.path.join(work_dir, "run.mac")
        with open(mac_path, "w") as f:
            f.write(mac_content)

        # Run Geant4
        cmd = [f"./{config.executable_name}", "run.mac"]
        result = subprocess.run(cmd, cwd=work_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        if result.returncode != 0:
            print(f"[Job {job_id}] FAILED! Return code: {result.returncode}")
            # print(f"[Job {job_id}] Stderr:\n{result.stderr}") # Optional: print stderr
            return False

        # Move output
        default_root = "CsI_Axion.root"  # This might need to be configurable too, but usually fixed in C++
        target_root = f"{config.output_prefix}{job_id}.root"

        src_root_path = os.path.join(work_dir, default_root)
        dst_root_path = os.path.join(config.build_dir, target_root)

        if os.path.exists(src_root_path):
            shutil.move(src_root_path, dst_root_path)
        else:
            print(f"[Job {job_id}] Warning: Output ROOT file not found!")
            return False

        # Handle ProcessIDMap (only for job 0)
        if job_id == 0:
            map_file = "ProcessIDMap.txt"
            src_map_path = os.path.join(work_dir, map_file)
            if os.path.exists(src_map_path):
                # Generate new map filename based on output filename
                base_name = os.path.splitext(config.output_filename)[0]
                new_map_name = f"{base_name}_ProcessIDMap.txt"
                dst_map_path = os.path.join(config.data_dir, new_map_name)

                shutil.move(src_map_path, dst_map_path)

        duration = time.time() - start_time
        # print(f"[Job {job_id}] Completed in {duration:.2f}s")
        return True

    except Exception as e:
        print(f"[Job {job_id}] Exception: {e}")
        return False

    finally:
        if os.path.exists(work_dir):
            shutil.rmtree(work_dir)


def merge_results(config):
    print("Merging files...")
    target_file_path = os.path.join(config.data_dir, config.output_filename)
    abs_target_path = os.path.abspath(target_file_path)

    output_files = [f"{config.output_prefix}{i}.root" for i in range(config.num_jobs)]
    merge_cmd = ["hadd", "-f", abs_target_path] + output_files

    try:
        subprocess.run(merge_cmd, cwd=config.build_dir, check=True)
        print(f"Successfully merged {len(output_files)} files into '{target_file_path}'")

        # Cleanup individual files
        # for f in output_files:
        #     os.remove(os.path.join(config.build_dir, f))

    except subprocess.CalledProcessError as e:
        print(f"Error merging files: {e}")
    except FileNotFoundError:
        print("Error: 'hadd' command not found. Please ensure ROOT is installed.")


def main():
    config = parse_arguments()

    if not check_environment(config):
        return

    print(f"Configuration:")
    print(f"  Output File: {config.output_filename}")
    print(f"  Jobs:        {config.num_jobs}")
    print(f"  Events/Job:  {config.events_per_job}")
    print(f"  Total Events: {config.num_jobs * config.events_per_job}")

    # Prepare arguments for workers
    worker_args = [(i, config) for i in range(config.num_jobs)]

    print(f"Starting {config.num_jobs} jobs...")
    with multiprocessing.Pool(processes=config.num_jobs) as pool:
        results = pool.map(run_single_job, worker_args)

    success_count = sum(results)
    print(f"\nSummary: {success_count}/{config.num_jobs} jobs succeeded.")

    if success_count == config.num_jobs:
        merge_results(config)
    else:
        print(f"Some jobs failed. Skipping merge.")


if __name__ == "__main__":
    main()
