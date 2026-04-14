from astroquery.gaia import Gaia
from astropy.table import Table
import numpy as np
from scipy.io import wavfile

job = Gaia.launch_job_async("""
SELECT TOP 1000000
    source_id,
    ra,
    dec,
    parallax,
    phot_g_mean_mag,
    phot_g_mean_flux,
    bp_rp,
    teff_gspphot,
    pmra,
    pmdec,
    radial_velocity,
    radial_velocity_error                
FROM gaiadr3.gaia_source
WHERE
    parallax > 7
    AND parallax_over_error > 10
""")
results = job.get_results()
print(type(results))
print(results)
print(len(results))

results.write("gaia_subset_parallax_7.fits", overwrite=True)