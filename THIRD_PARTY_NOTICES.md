# Third-party notices

## Scope and distribution

Stellaverb's original source code is licensed under AGPL-3.0-only; see [LICENSE](LICENSE).
Third-party software and astronomical data retain their own terms. This document
records the JUCE 8.0.12 dependency notices collected for the macOS VST3/AU release
configuration. It is not a claim that every listed library is linked into every
format, or that these notices cover other platforms or future configurations.

Include this file, `LICENSE`, and the `licenses/` directory in plugin release
archives. Provide the corresponding source for each AGPL binary release, including
the dependency sources and build materials required by the licence. Notices alone
do not replace that source-distribution obligation. Review the inventory when
updating JUCE, adding a platform, or enabling additional modules/codecs.

## JUCE and bundled components

Stellaverb uses JUCE 8.0.12 under its AGPLv3 licensing option.
JUCE is copyright Raw Material Software Limited. The JUCE modules also contain
third-party components with their own notices. This software is based in part on
the work of the Independent JPEG Group.

The files below are unmodified copies from the local JUCE 8.0.12 source tree.
Paths in the copied JUCE overview refer to the upstream JUCE tree, not this repository.
See the [upstream licence overview](https://github.com/juce-framework/JUCE/blob/8.0.12/LICENSE.md).

| Component | Included notice | Source within JUCE |
|---|---|---|
| JUCE | [JUCE-LICENSE.md](licenses/juce-8.0.12/JUCE-LICENSE.md) | `LICENSE.md` |
| VST3 SDK | [VST3-LICENSE.txt](licenses/juce-8.0.12/VST3-LICENSE.txt) | `modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt` |
| AudioUnitSDK | [AudioUnitSDK-LICENSE.txt](licenses/juce-8.0.12/AudioUnitSDK-LICENSE.txt) | `modules/juce_audio_plugin_client/AU/AudioUnitSDK/LICENSE.txt` |
| FLAC | [FLAC-LICENSE.txt](licenses/juce-8.0.12/FLAC-LICENSE.txt) | `modules/juce_audio_formats/codecs/flac/Flac Licence.txt` |
| Ogg Vorbis | [OggVorbis-LICENSE.txt](licenses/juce-8.0.12/OggVorbis-LICENSE.txt) | `modules/juce_audio_formats/codecs/oggvorbis/Ogg Vorbis Licence.txt` |
| libvorbis | [libvorbis-COPYING.txt](licenses/juce-8.0.12/libvorbis-COPYING.txt) | `modules/juce_audio_formats/codecs/oggvorbis/libvorbis-1.3.7/COPYING` |
| Independent JPEG Group | [JPEG-README.txt](licenses/juce-8.0.12/JPEG-README.txt) | `modules/juce_graphics/image_formats/jpglib/README` |
| libpng | [libpng-LICENSE.txt](licenses/juce-8.0.12/libpng-LICENSE.txt) | `modules/juce_graphics/image_formats/pnglib/LICENSE` |
| zlib | [zlib-LICENSE.txt](licenses/juce-8.0.12/zlib-LICENSE.txt) | `modules/juce_core/zip/zlib/LICENSE` |
| HarfBuzz | [HarfBuzz-COPYING.txt](licenses/juce-8.0.12/HarfBuzz-COPYING.txt) | `modules/juce_graphics/fonts/harfbuzz/COPYING` |
| SheenBidi | [SheenBidi-LICENSE.txt](licenses/juce-8.0.12/SheenBidi-LICENSE.txt) | `modules/juce_graphics/unicode/sheenbidi/LICENSE` |

## Gaia DR3 data

Credit: **ESA/Gaia/DPAC**.

Stellaverb uses a processed subset of the `gaiadr3.gaia_source` catalogue.
ESA's Gaia DR3 documentation states that Gaia data are open and free to use with
credit to ESA/Gaia/DPAC. These data are not relicensed under Stellaverb's AGPL notice.
See the [official data-use, acknowledgement and citation instructions](https://gea.esac.esa.int/archive/documentation/GDR3/Miscellaneous/sec_credit_and_citation_instructions/).

### Acknowledgement

This work has made use of data from the European Space Agency (ESA) mission Gaia (https://www.cosmos.esa.int/gaia), processed by the Gaia Data Processing and Analysis Consortium (DPAC, https://www.cosmos.esa.int/web/gaia/dpac/consortium). Funding for the DPAC has been provided by national institutions, in particular the institutions participating in the Gaia Multilateral Agreement.

### References

- Gaia Collaboration et al. (2016), *The Gaia mission*.
- Gaia Collaboration et al. (2023), *Gaia Data Release 3: Summary of the content and survey properties*.

The official credit instructions linked above provide the bibliographic links and
additional papers relevant to specific Gaia data products.

### Processing and provenance

`astropyQueryAndSave.py` queries up to 1,000,000 Gaia DR3 rows with
`parallax > 7` and `parallax_over_error > 10`, saving the result as
`gaia_subset_parallax_7.fits`. `astropyPreprocessing.py` sorts by the distance
estimate `1000 / parallax`, selects up to 100,000 stars, and derives normalized
distance, brightness, stereo weights, azimuth, and Cartesian directions.
The packed `stars_sorted.bin` data contain eight little-endian float32 values per
star: left, right, amplitude, distance, azimuth, x, y, and z. The plugin embeds
`Stellaverb/Source/stars_sorted.bin`.

These are transformations for creative audio mapping, not an acoustic measurement
of space. Processing scripts depend on separately installed Python packages;
those packages are not bundled with the plugin by this repository.

## Inspiration

Inspired by [Katsuhiro Chiba's Stellaverb](https://audiooo.com/stellaverb).
This is an independent implementation by Adrian Lam and does not include Chiba's
original patch or source code. This acknowledgement does not imply endorsement.
