import numpy as np
import matplotlib.pyplot as plt


def read_input(path="input.txt"):
    with open(path, "r") as f:
        lines = [ln.strip() for ln in f if ln.strip() and not ln.strip().startswith("#")]

    maxperf = float(lines[0])  # MFLOP/s
    maxband = float(lines[1])  # MB/s  (ou "MBytes/s" cohérent avec ta formule)

    points = []
    for ln in lines[2:]:
        parts = ln.split()
        if len(parts) < 3:
            continue
        ai = float(parts[0])
        perf = float(parts[1])
        label = " ".join(parts[2:])
        points.append((ai, perf, label))

    return maxperf, maxband, points


def roofline_curve(maxperf, maxband, xmin, xmax, n=300):
    """
    maxperf: MFLOP/s
    maxband: (MFLOP/s) per (FLOP/Byte)? Non:
      Ici on suppose la formule perf = AI * maxband, donc maxband doit être en (MFLOP/s) / (FLOP/Byte)
      Autrement dit, maxband doit être en MBytes/s si AI est FLOP/Byte et perf en MFLOP/s:
         perf[MFLOP/s] = AI[FLOP/Byte] * maxband[MByte/s]
      (c'est cohérent avec ton modèle)
    """
    xs = np.logspace(np.log10(xmin), np.log10(xmax), n)
    ys = np.minimum(maxperf, xs * maxband)
    return xs, ys


def annotate_points(ax, pts, min_dx_frac=0.03, min_dy_frac=0.03):
    """
    Petit 'repel' simple:
    - On place des offsets alternés
    - Si une annotation retombe trop près d'une autre, on décale un peu plus.
    (Pas aussi parfait que adjustText, mais ça aide beaucoup sans dépendance.)
    """
    if not pts:
        return

    # tailles relatives pour décaler
    xlim = ax.get_xlim()
    ylim = ax.get_ylim()
    dx0 = (np.log10(xlim[1]) - np.log10(xlim[0])) * min_dx_frac
    dy0 = (np.log10(ylim[1]) - np.log10(ylim[0])) * min_dy_frac

    placed = []  # positions des labels en log10
    pattern = [(+1, +1), (+1, -1), (-1, +1), (-1, -1), (+2, +1), (+2, -1), (-2, +1), (-2, -1)]

    for i, (x, y, label) in enumerate(pts):
        if x <= 0 or y <= 0:
            continue

        lx, ly = np.log10(x), np.log10(y)
        sx, sy = pattern[i % len(pattern)]
        offx, offy = sx * dx0, sy * dy0

        # éviter collisions naïvement
        for _ in range(12):
            ok = True
            for px, py in placed:
                if abs((lx + offx) - px) < dx0 * 0.9 and abs((ly + offy) - py) < dy0 * 0.9:
                    ok = False
                    break
            if ok:
                break
            offx *= 1.25
            offy *= 1.25

        placed.append((lx + offx, ly + offy))

        ax.annotate(
            label,
            xy=(x, y),
            xytext=(10 * offx, 10 * offy),  # en points, effet visuel
            textcoords="offset points",
            fontsize=9,
            arrowprops=dict(arrowstyle="-", lw=0.8),
        )


def plot_roofline(input_path="input.txt"):
    maxperf, maxband, points = read_input(input_path)

    # filtrer points positifs
    pts = [(ai, p, lab) for (ai, p, lab) in points if ai > 0 and p > 0]

    # bornes x/y (log) basées sur les points + roofline
    if pts:
        ais = np.array([p[0] for p in pts])
        perfs = np.array([p[1] for p in pts])
        xmin = max(ais.min() * 0.5, 1e-3)
        xmax = max(ais.max() * 2.0, 10.0)
        ymin = max(perfs.min() * 0.5, 1e-1)
        ymax = max(maxperf * 1.3, perfs.max() * 2.0)
    else:
        xmin, xmax = 1e-3, 100
        ymin, ymax = 1e-1, maxperf * 1.3

    # roofline
    xs, ys = roofline_curve(maxperf, maxband, xmin, xmax)

    fig, ax = plt.subplots(figsize=(12, 5))

    # échelles log (indispensable pour lisibilité)
    ax.set_xscale("log")
    ax.set_yscale("log")

    # Tracer roofline
    ax.plot(xs, ys, lw=2, label="Roofline")

    # Marquer le "knee"
    knee_x = maxperf / maxband if maxband > 0 else np.nan
    if np.isfinite(knee_x) and knee_x > 0:
        ax.axvline(knee_x, ls="--", lw=1, alpha=0.8, label=f"Knee (AI={knee_x:.3g})")

    # Points
    if pts:
        ax.scatter([p[0] for p in pts], [p[1] for p in pts], s=40)
        annotate_points(ax, pts)

    ax.set_xlim(xmin, xmax)
    ax.set_ylim(ymin, ymax)

    ax.grid(True, which="both", ls=":", lw=0.8)
    ax.set_xlabel("Arithmetic Intensity [FLOP/Byte]")
    ax.set_ylabel("Performance [MFLOP/s]")
    ax.set_title("Roofline model")

    # mini résumé dans la légende
    ax.legend(loc="best", fontsize=9)

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    plot_roofline("input.txt")
