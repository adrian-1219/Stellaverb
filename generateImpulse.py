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

# quit()

sr = 44100
delay_time = 5 # in seconds

tbl = Table.read("gaia_subset_parallax_7.fits")
tbl['distance_pc'] = 1000.0 / tbl['parallax']
tbl.sort('distance_pc')

def scale(value, old_min, old_max, new_min, new_max):
    # avoid division by zero
    if old_min == old_max:
        return (new_min + new_max) / 2.0

    t = (value - old_min) / (old_max - old_min)
    return new_min + t * (new_max - new_min)

irLenSamples = sr * delay_time
irBuffer = np.zeros(irLenSamples)

# lower phot_g_mean_mag means higher brightness
# min_brightness = tbl['phot_g_mean_mag'].max()
# max_brightness = tbl['phot_g_mean_mag'].min()

max_distance = tbl['distance_pc'].max()
min_distance = tbl['distance_pc'].min()

for row in tbl:
    if np.ma.is_masked(row['phot_g_mean_flux']) or np.ma.is_masked(row['distance_pc']):
        continue

    # value = row['phot_g_mean_mag']
    # if not np.isnan(value):
    index = int(scale(row['distance_pc'], min_distance, max_distance, 0, irLenSamples - 1))

    # TODO: map to stereo and make it so that it adds 
    irBuffer[index] += row['phot_g_mean_flux']

max_sample_value = irBuffer.max()
for i in range(len(irBuffer)):
    irBuffer[i] = float(scale(irBuffer[i], 0, max_sample_value, 0, 1))
irBufferReversed = irBuffer[::-1]

# print(irBuffer.tolist())
# print(min_brightness, max_brightness)
# print(min_distance, max_distance)

irBufferReversed = np.nan_to_num(irBufferReversed, nan=0.0, posinf=0.0, neginf=0.0)
wavfile.write("ir.wav", sr, (irBufferReversed * 32767).astype(np.int16))