#!/usr/bin/env python3
"""Generate the Zensors icon artwork as a self-contained SVG.

    ./scripts/make-icon.py [output.svg]      (default: icons/zensors.svg)

The mark is an enso - the Zen circle - drawn as a single brush stroke that
swells through the stroke and closes to a point, left open at the upper
right. A sensor point sits at its centre and two arcs radiate from it
through that opening: stillness at the core, sensing outward.

Two constraints shape the implementation:

  * ImageMagick's built-in SVG renderer ignores stroke=, so every stroke is
    emitted as a FILLED outline instead.
  * The same renderer handles fills reliably, so the radial background is
    baked in as concentric discs, keeping the SVG a single file that renders
    identically at any size.

Segmenting each stroke into overlapping quads is also what buys the colour
ramp along the brush.
"""
import math
import os
import sys

W = H = 512
CX = CY = 256.0

# Palette: warm coral on a cool near-black, matching the app's own colours
EDGE = (0x0A, 0x0D, 0x12)
MID = (0x2B, 0x34, 0x40)
BRUSH_FROM = (0xFF, 0x9E, 0x70)
BRUSH_TO = (0xEE, 0x4A, 0x1F)
WAVE_A = ((0xFF, 0xB2, 0x8E), (0xFF, 0x8A, 0x5C))
WAVE_B = ((0xFF, 0x9A, 0x70), (0xFF, 0x6A, 0x3C))
CORE = "#FFEAD9"

R0 = 188.0        # mean radius of the enso
GAP = 56.0        # width of its opening, in degrees
GAP_MID = -45.0   # ... centred on the upper right


def lerp(a, b, t):
    return a + (b - a) * t


def smoothstep(t):
    t = max(0.0, min(1.0, t))
    return t * t * (3 - 2 * t)


def hexc(c):
    return "#%02x%02x%02x" % tuple(int(round(v)) for v in c)


def ramp(c0, c1, t):
    return hexc([lerp(c0[i], c1[i], t) for i in range(3)])


def disc(cx, cy, r, color, n=96):
    pts = [(cx + r * math.cos(2 * math.pi * i / n),
            cy + r * math.sin(2 * math.pi * i / n)) for i in range(n)]
    d = "M %.2f %.2f" % pts[0] + "".join(" L %.2f %.2f" % p for p in pts[1:]) + " Z"
    return '  <path d="%s" fill="%s"/>' % (d, color)


def stroke(cx, cy, r_of, w_of, a0, a1, col0, col1, nseg, over):
    """A tapered stroke as nseg overlapping quads, coloured along its length."""
    parts = []
    for i in range(nseg):
        t0, t1 = i / nseg, (i + 1) / nseg
        th0 = math.radians(lerp(a0, a1, t0))
        th1 = math.radians(lerp(a0, a1, t1))
        # Widen along the sweep direction, whichever way that happens to run,
        # so neighbouring segments overlap instead of leaving white seams.
        pad = math.radians(over) * (1 if th1 >= th0 else -1)
        th0p, th1p = th0 - pad, th1 + pad
        tm = (t0 + t1) / 2
        r, w = r_of(tm), w_of(tm)
        ro, ri = r + w / 2, r - w / 2
        pts = [(cx + ro * math.cos(t), cy + ro * math.sin(t)) for t in (th0p, th1p)]
        pts += [(cx + ri * math.cos(t), cy + ri * math.sin(t)) for t in (th1p, th0p)]
        d = "M %.2f %.2f" % pts[0] + "".join(" L %.2f %.2f" % p for p in pts[1:]) + " Z"
        parts.append('  <path d="%s" fill="%s"/>' % (d, ramp(col0, col1, tm)))
    return parts


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "..", "icons", "zensors.svg")

    art = []

    # Radial background, drawn largest disc first so each covers the last.
    rmax = math.hypot(W / 2, H / 2)
    nbg = 160
    for i in range(nbg, -1, -1):
        t = (1 - i / nbg) ** 0.85      # ease: bright core, quick falloff
        art.append(disc(CX, CY, max(rmax * i / nbg, 0.01), ramp(EDGE, MID, t)))

    # The enso: a slight hand-drawn wobble in the radius, a brush that is
    # thin where it starts, swells through the stroke and closes to a point.
    # The stroke runs a_start -> a_end in decreasing angle, so the opening
    # it leaves is (a_start, a_end): putting a_end on the far side of the
    # mark's direction centres that opening on the waves.
    a_end = GAP_MID + GAP / 2
    a_start = a_end + (360.0 - GAP)

    def r_of(t):
        a = math.radians(lerp(a_start, a_end, t))
        return R0 * (1 + 0.016 * math.sin(1.7 * a + 1.1)
                       + 0.009 * math.sin(3.3 * a + 0.4))

    def w_of(t):
        if t < 0.42:
            return 14 + 34 * smoothstep(t / 0.42)
        return 48 - 44 * smoothstep((t - 0.42) / 0.58)

    art += stroke(CX, CY, r_of, w_of, a_start, a_end,
                  BRUSH_FROM, BRUSH_TO, nseg=128, over=0.7)

    # Two sensing arcs, aimed out through the opening. Their ends are
    # rounded by tapering the width with sqrt(sin), not by a line cap.
    for radius, width, (c0, c1), half in ((94, 22, WAVE_A, 34.0),
                                          (140, 17, WAVE_B, 34.0)):
        art += stroke(CX, CY,
                      lambda t, r=radius: r,
                      lambda t, w=width: w * math.sqrt(math.sin(math.pi * max(1e-3, min(1, t)))),
                      GAP_MID - half, GAP_MID + half, c0, c1, nseg=40, over=1.4)

    # The sensor point itself.
    art.append(disc(CX, CY, 31, CORE))

    svg = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" '
           'viewBox="0 0 %d %d">' % (W, H, W, H)]
    svg += art
    svg.append("</svg>")

    out = os.path.abspath(out)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w") as fh:
        fh.write("\n".join(svg) + "\n")
    print("wrote %s (%d paths)" % (out, len(art)))


if __name__ == "__main__":
    main()
