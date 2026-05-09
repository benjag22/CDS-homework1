import os
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

PLOTS_DIR = "plots"
os.makedirs(PLOTS_DIR, exist_ok=True)

# Define different color palettes to distinct datasets
PALETTES = ["Blues", "Greens", "Oranges", "Purples", "Reds", "Greys"]


def is_brute_force(algo_name: str) -> bool:
    return "brute" in algo_name.lower() or "bf" in algo_name.lower()


def plot_execution_time(df: pd.DataFrame, exclude_bf: bool = False) -> None:
    plot_df = df.copy()
    suffix = ""
    title_suffix = ""

    if exclude_bf:
        plot_df = plot_df[~plot_df["type"].apply(is_brute_force)]
        suffix = "_no_bf"
        title_suffix = " (Excluding Brute Force)"

    plt.figure(figsize=(14, 8))

    datasets = plot_df["dataset"].unique()

    for dt_idx, dataset_name in enumerate(datasets):
        ds_df = plot_df[plot_df["dataset"] == dataset_name]
        algo_types = ds_df["type"].unique()

        # Pick a colormap palette for this specific dataset
        cmap = plt.get_cmap(PALETTES[dt_idx % len(PALETTES)])

        # Generate slightly differing colors from the chosen palette
        colors = [cmap(i) for i in np.linspace(0.4, 0.9, len(algo_types))]

        for algo_idx, algo_type in enumerate(algo_types):
            algo_df = ds_df[ds_df["type"] == algo_type]

            plt.errorbar(
                algo_df["n"], algo_df["t_mean"], yerr=algo_df["t_stdev"],
                fmt="o-", capsize=4, markersize=5,
                color=colors[algo_idx],
                label=f"{dataset_name} - {algo_type}",
                alpha=0.85
            )

    plt.title(f"Execution Time vs Pattern Size Across Datasets{title_suffix}", fontsize=14)
    plt.xlabel("Pattern Size (n)", fontsize=12)
    plt.ylabel("Execution Time (ns)", fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.6)

    # Place legend inside, on the top left
    plt.legend(loc="upper left", fontsize=10)
    plt.tight_layout()

    out_path = os.path.join(PLOTS_DIR, f"execution_time_combined{suffix}.png")
    plt.savefig(out_path, dpi=300)
    plt.close()
    print(f"Saved {out_path}")


def plot_memory_sizes(df: pd.DataFrame) -> None:
    # Since memory usage per data structure size is static across 'n', drop duplicates
    mem_df = df.drop_duplicates(subset=["dataset", "type"])[["dataset", "type", "mem"]]

    # Pivot the data to group natively: index=algorithm type, columns=dataset
    pivot_df = mem_df.pivot(index="type", columns="dataset", values="mem")

    ax = pivot_df.plot(kind="bar", figsize=(12, 7), width=0.8, alpha=0.85)

    plt.title("Data Structure Size in Memory by Algorithm", fontsize=14)
    plt.xlabel("Algorithm Type", fontsize=12)
    plt.ylabel("Memory Usage (bytes)", fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.5, axis='y')
    plt.xticks(rotation=0)
    plt.legend(title="Dataset")

    for container in ax.containers:
        ax.bar_label(container, fmt='%d', label_type='edge', padding=2, rotation=90, fontsize=9)

    y_max = pivot_df.max().max() if not pivot_df.empty else 1
    plt.ylim(0, y_max * 1.3)

    plt.tight_layout()
    out_path = os.path.join(PLOTS_DIR, "memory_size_combined.png")
    plt.savefig(out_path, dpi=300)
    plt.close()
    print(f"Saved {out_path}")


def plot_individual_datasets(df: pd.DataFrame, exclude_bf: bool = False) -> None:
    plot_df = df.copy()
    suffix = ""
    title_suffix = ""

    if exclude_bf:
        plot_df = plot_df[~plot_df["type"].apply(is_brute_force)]
        suffix = "_no_bf"
        title_suffix = " (Excluding Brute Force)"

    datasets = plot_df["dataset"].unique()

    for dataset_name in datasets:
        ds_df = plot_df[plot_df["dataset"] == dataset_name]

        # Skip if empty (for instance, if a dataset only contained brute force)
        if ds_df.empty:
            continue

        plt.figure(figsize=(10, 6))

        # ---------------------------------------------------------
        # Execution Time
        # ---------------------------------------------------------
        types = ds_df["type"].unique()
        for algo_type in types:
            algo_df = ds_df[ds_df["type"] == algo_type]
            plt.errorbar(
                algo_df["n"], algo_df["t_mean"], yerr=algo_df["t_stdev"],
                fmt="o-", capsize=5, markersize=6, label=algo_type, alpha=0.8
            )

        plt.title(f"{dataset_name} Dataset - Execution Time vs Pattern Size{title_suffix}", fontsize=14)
        plt.xlabel("Pattern Size (n)", fontsize=12)
        plt.ylabel("Execution Time (ns)", fontsize=12)
        plt.grid(True, linestyle="--", alpha=0.7)
        plt.legend(fontsize=11)

        plt.tight_layout()
        out_path = os.path.join(PLOTS_DIR, f"{dataset_name.lower()}_analysis{suffix}.png")
        plt.savefig(out_path, dpi=300)
        plt.close()

        print(f"Saved {out_path}")


def main():
    data_dir = "data"

    if not os.path.exists(data_dir):
        print(f"Error: Directory '{data_dir}' not found.")
        return

    csv_files = list(Path(data_dir).glob("*.csv"))

    if not csv_files:
        print(f"No CSV files found in '{data_dir}' directory.")
        return

    print(f"Found {len(csv_files)} CSV files. Loading data...")

    dataframes = []
    for f in csv_files:
        df = pd.read_csv(f)
        if not df.empty and "type" in df.columns:
            df["dataset"] = f.stem.title()
            dataframes.append(df)

    if not dataframes:
        print("No valid data to plot.")
        return

    combined_df = pd.concat(dataframes, ignore_index=True)

    print("Generating Execution Time plots...")
    plot_execution_time(combined_df, exclude_bf=False)
    plot_execution_time(combined_df, exclude_bf=True)

    print("Generating Combined Memory Size plot...")
    plot_memory_sizes(combined_df)

    print("Generating Individual Dataset plots...")
    plot_individual_datasets(combined_df, exclude_bf=False)
    plot_individual_datasets(combined_df, exclude_bf=True)

    print("Processing complete.")


if __name__ == "__main__":
    main()
