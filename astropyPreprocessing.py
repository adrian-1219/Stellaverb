from astroquery.gaia import Gaia
from astropy.table import Table
import numpy as np
from scipy.io import wavfile

# job = Gaia.launch_job_async("""
# SELECT TOP 1000000
#     source_id,
#     ra,
#     dec,
#     parallax,
#     phot_g_mean_mag,
#     phot_g_mean_flux,
#     bp_rp,
#     teff_gspphot,
#     pmra,
#     pmdec,
#     radial_velocity,
#     radial_velocity_error                
# FROM gaiadr3.gaia_source
# WHERE
#     parallax > 7
#     AND parallax_over_error > 10
# """)
# results = job.get_results()
# print(type(results))
# print(results)
# print(len(results))

# results.write("gaia_subset_parallax_7.fits", overwrite=True)

MAX_STARS = 1000000          # final dataset size

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

# precompute azimuth for panning
azimuth = np.arctan2(y, x)

pan = np.sin(azimuth)  # -1..1

left  = np.sqrt(0.5 * (1 - pan))
right = np.sqrt(0.5 * (1 + pan))

# Stack → float32
data = np.stack([left, right, amplitude, distance, azimuth], axis=1).astype('<f4')

# Save as binary for performance 
np.asarray(data).astype('<f4').tofile("stars_sorted.bin")

# Verify it saved correctly
data = np.fromfile("stars_sorted.bin", dtype='<f4')
print(data.shape)  # should be N * 5
data = data.reshape((-1, 5))
print(data[:5])  # inspect first few rows

print(np.min(left), np.max(left))
print(np.min(right), np.max(right))