from astroquery.gaia import Gaia
from astropy.table import Table
import numpy as np
from scipy.io import wavfile

# disable scientific notation 
np.set_printoptions(suppress=True, precision=8)

MAX_STARS = 100000         

tbl = Table.read("gaia_subset_parallax_7.fits")
tbl['distance_pc'] = 1000.0 / tbl['parallax']
tbl.sort('distance_pc')
tbl = tbl[:MAX_STARS]

# distance
distance = tbl['distance_pc']
# normalize distance so we can scale it to the desired length easily when generating the impulse response
d_max = np.max(distance)
distance = distance / d_max 

distance = np.clip(distance, 0.0, 1.0)
distance = np.nan_to_num(distance, nan=0.0, posinf=1.0, neginf=0.0)

# convert ra and dec to cartesian coordinates
ra = np.deg2rad(tbl['ra'])
dec = np.deg2rad(tbl['dec'])

x = np.cos(dec) * np.cos(ra)
y = np.cos(dec) * np.sin(ra)
z = np.sin(dec)

x = (x + 1.0) * 0.5
y = (y + 1.0) * 0.5
z = (z + 1.0) * 0.5

# convert magnitudes to amplitudes
mag = tbl['phot_g_mean_mag']
amplitude = 10 ** (-0.4 * mag)
# remove outliers first
lo, hi = np.percentile(amplitude, (1, 99))
amplitude = np.clip(amplitude, lo, hi)
# normalize to 0–1 evenly
amplitude = (amplitude - lo) / (hi - lo)
amplitude = np.nan_to_num(amplitude, nan=0.0, posinf=0.0, neginf=0.0)

mask = (
    np.isfinite(x) &
    np.isfinite(y) &
    np.isfinite(z) &
    np.isfinite(amplitude) &
    np.isfinite(distance)
)

x = x[mask]
y = y[mask]
z = z[mask]
amplitude = amplitude[mask]
distance = distance[mask]

print("Brightness dist", np.percentile(amplitude, [0, 25, 50, 75, 100]))

# precompute azimuth for panning
azimuth = np.arctan2(y, x)

# downsample for GUI
downsample_factor = 100
x = x[::downsample_factor]
y = y[::downsample_factor]
z = z[::downsample_factor]
amplitude = amplitude[::downsample_factor]
distance = distance[::downsample_factor]
azimuth = azimuth[::downsample_factor]

# filter out dim stars for GUI
# brightness_threshold = np.percentile(amplitude, 99)

# bright_mask = amplitude >= brightness_threshold

# x = x[bright_mask]
# y = y[bright_mask]
# z = z[bright_mask]
# amplitude = amplitude[bright_mask]
# distance = distance[bright_mask]
# azimuth = azimuth[bright_mask]

print("Number of stars:", len(x))

# Stack → float32
data = np.stack([x, y, z, amplitude, distance, azimuth], axis=1).astype('<f4')

# Save as binary for performance 
np.asarray(data).astype('<f4').tofile("stars_sorted_GUI.bin")

# Verify it saved correctly
data = np.fromfile("stars_sorted_GUI.bin", dtype='<f4')
print(data.shape)  # should be N * 6
data = data.reshape((-1, 6))
print(data[:5])  # inspect first few rows
