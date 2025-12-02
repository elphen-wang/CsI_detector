import awkward as ak
import numpy as np
import uproot

try:
    file = uproot.open("build/CsI_Axion.root")
    tree = file["CsI_Axion"]

    # Check if branches exist
    print("Branches:", tree.keys())

    if "PhotonExitCount" in tree.keys():
        counts = tree["PhotonExitCount"].array()
        ids = tree["PhotonExitCrystalID"].array()

        print(f"Total events: {len(counts)}")

        non_zero_events = 0
        total_photons = 0

        for i in range(len(counts)):
            if len(counts[i]) > 0:
                non_zero_events += 1
                total_photons += np.sum(counts[i])
                if i < 5:  # Print first 5 non-empty events
                    print(f"Event {i}: IDs={ids[i]}, Counts={counts[i]}")

        print(f"Events with exiting photons: {non_zero_events}")
        print(f"Total exiting photons detected: {total_photons}")

    else:
        print("Branch 'PhotonExitCount' not found!")

except Exception as e:
    print(f"Error: {e}")
