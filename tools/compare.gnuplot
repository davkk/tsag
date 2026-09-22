# tools/compare.gnuplot — overlay RSS curves from bench runs.
# Usage:
#   gnuplot -e "FILES='a/samples.csv b/samples.csv'; LABELS='a b'; OUT='bench/compare.png'" tools/compare.gnuplot
# or via the wrapper: tools/plot_compare.sh [--out compare.png] <benchdir|samples.csv> [...]
# (Filenames containing spaces are not supported; use the wrapper, which also
#  prints the peak table.)
if (!exists("FILES")) FILES = ""
if (!exists("LABELS")) LABELS = ""
if (!exists("OUT")) OUT = "bench/compare.png"

set datafile separator comma
set terminal pngcairo size 1280,640 font ',10'
set output OUT
set xlabel 'time (s)'
set ylabel 'RSS total (MB)'
set grid
set key outside right top

# col 1 = t_s, col 3 = total_mb; every ::1 skips the CSV header row
plot for [i=1:words(FILES)] word(FILES,i) every ::1 using 1:3 \
  with lines linewidth 1.5 title word(LABELS,i)
