import json
import argparse
import matplotlib.pyplot as plt
import numpy as np
from scipy.io import loadmat


def main():
    parser = argparse.ArgumentParser(
        description="Visualisation ECG (.mat) avec détection des pics P, Q, R, S, T"
    )

    parser.add_argument(
        "--mat",
        required=True,
        help="Chemin vers le fichier MAT ECG"
    )
    parser.add_argument(
        "--json",
        required=True,
        help="Chemin vers le fichier JSON des pics"
    )
    parser.add_argument(
        "--lead",
        default="lead2",
        help="Nom du lead à afficher (ex: lead1, lead2, ..., lead12)"
    )
    parser.add_argument(
        "--samples",
        type=int,
        default=5000,
        help="Nombre d'échantillons à afficher (par défaut: 5000)"
    )

    args = parser.parse_args()

    # ================================
    # Charger le signal ECG depuis .mat
    # ================================
    mat_data = loadmat(args.mat)

    if "val" not in mat_data:
        raise ValueError("Variable 'val' introuvable dans le fichier .mat")

    ecg_matrix = np.asarray(mat_data["val"], dtype=np.float64)

    # On attend une matrice 2D
    if ecg_matrix.ndim != 2:
        raise ValueError("La variable 'val' n'est pas une matrice 2D")

    # Gestion orientation (LEADS x N) ou (N x LEADS)
    if ecg_matrix.shape[0] <= ecg_matrix.shape[1]:
        leads, n_samples_total = ecg_matrix.shape
    else:
        ecg_matrix = ecg_matrix.T
        leads, n_samples_total = ecg_matrix.shape

    # Convertir "lead2" → index 1
    if not args.lead.lower().startswith("lead"):
        raise ValueError("Format attendu: leadX (ex: lead2)")

    lead_index = int(args.lead[4:]) - 1

    if lead_index < 0 or lead_index >= leads:
        raise ValueError(f"Lead '{args.lead}' invalide (max = lead{leads})")

    ecg_signal = ecg_matrix[lead_index, :args.samples]

    # ================================
    # Charger le JSON des pics
    # ================================
    with open(args.json, "r") as f:
        data = json.load(f)

    peaks = data["peaks"]

    def filter_peaks(name):
        return [p for p in peaks.get(name, []) if p < args.samples]

    p_peaks = filter_peaks("P")
    q_peaks = filter_peaks("Q")
    r_peaks = filter_peaks("R")
    s_peaks = filter_peaks("S")
    t_peaks = filter_peaks("T")

    # ================================
    # Plot
    # ================================
    plt.figure(figsize=(14, 6))
    plt.plot(ecg_signal, label=f"ECG ({args.lead})", color="blue")

    plt.scatter(p_peaks, ecg_signal[p_peaks], color="orange", label="P", zorder=3)
    plt.scatter(q_peaks, ecg_signal[q_peaks], color="purple", label="Q", zorder=3)
    plt.scatter(r_peaks, ecg_signal[r_peaks], color="red", label="R", zorder=3)
    plt.scatter(s_peaks, ecg_signal[s_peaks], color="green", label="S", zorder=3)
    plt.scatter(t_peaks, ecg_signal[t_peaks], color="magenta", label="T", zorder=3)

    plt.xlabel("Temps (échantillons)")
    plt.ylabel("Amplitude ECG")
    plt.title("ECG (.mat) avec détection des pics P, Q, R, S, T")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
