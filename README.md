
## Stellaverb

Inspired by [Katsuhiro Chiba's Max MSP patch](https://audiooo.com/stellaverb), Stellaverb is a convolution reverb plugin that transforms astronomical data into unique reverberation textures. Drawing from the ESA Gaia Star Catalogue, it explores the idea of simulating how sound might propagate to distant stars and return, while also enabling more abstract, creative reverbs shaped by stellar properties such as distance, brightness, and position.

This project was developed as the final assignment for the ECS7012 Music and Audio Programming module at Queen Mary University of London.

## Demo Video

Here is a short demo video of what the plugin sounds like.

https://github.com/user-attachments/assets/cb92aee9-d773-4f54-8f14-84a707983d79

## Install
TODO

## Features

The core of the plugin is its ability to generate an impulse response (IR) using the star data based on the user's settings, then convolving the input signal with the generated IR. Each star is mapped to a sample in the IR based on its distance, position and brightness.

The plugin has 3 modes, 5 IR generation parameters, and 7 post-processing effects.

![Screenshot of UI](./assets/ui.png)

### Modes
Modes are different mapping algorithms used when converting star data into impulse responses.
- Acoustic
    - Meant to simulate the acoustic effect of sound propagating to the stars and back
    - Distance of each star is mapped to delay time and attenuation, closer stars are louder while further stars are quieter
    - Position of each star is mapped to stereo panning
    - Same as original Stellaverb Max MSP patch
- Reverse:
    - Opposite of Acoustic where further stars are louder and closer stars are quieter
    - Distance of each star is mapped to delay time and attenuation, closer stars are quieter while further stars are louder
    - Position of each star is mapped to stereo panning
- Sustain:
    - Brightness of each star is mapped to amplitude (loudness)
    - Distance of each star is mapped to delay time
    - Position of each star is mapped to stereo panning

### IR Generation Parameters
These are parameters that control which stars are used when generating the IR and the length of the generated IR.
- Length - length of the generated IR in seconds
- Min Distance - minimum distance of the stars used in IR generation (stars closer than Min Distance will be filtered out)
- Max Distance - maximum distance of the stars used in IR generation
- Min Brightness - minimum brightness of the stars used in IR generation
- Max Brightness - maximum brightness of the stars used in IR generation

### Other Processing Effects
These are standard processing effects found in most reverb plugins.
- Pre-delay - controls the amount of time between the original dry sound and the onset of the reverb effect
- Highpass Cutoff - controls the threshold where the frequencies below it begins to be reduced in amplitude
- Lowpass Cutoff - controls the threshold where the frequencies above it begins to be reduced in amplitude
- Drive - controls the amount of drive in tanh saturation
- Width - controls the amount of mid side widening
- Dry/Wet - controls the balance between the original signal and the convolved signal
- Gain - controls the gain of the final output signal

## Build Instructions

Use JUCE 8.0.12 Projucer and Xcode (the packaged binaries used Xcode 16.3).
JUCE 8.0.12 including Projucer source:
https://github.com/juce-framework/JUCE/tree/8.0.12
https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.12.zip

1. Open Source/Stellaverb/Stellaverb.jucer in Projucer 8.0.12.
2. Set Projucer's global JUCE modules path to Source/JUCE/modules in this
   extracted package. The project uses global module paths, so this overrides
   any developer-specific paths stored in the project.
3. Save/export the Xcode project and open Builds/MacOSX/Stellaverb.xcodeproj.
4. Build the VST3 and AU schemes in Release for arm64, targeting macOS 11.0.
5. Products are written to Builds/MacOSX/build/Release under the project.

The star data and presets are already included. The Python scripts are only
needed to recreate the processed dataset, not to build the plugin.

## License

Stellaverb - Convolution plugin that transforms astronomical data into reverberation textures
Copyright (C) 2026 Adrian Lam

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

Stellaverb uses **JUCE 8.0.12 under its AGPLv3 option**. Third-party software
and data retain their own terms. See [LICENSE](LICENSE) and
[Third-party notices and Gaia data provenance](THIRD_PARTY_NOTICES.md).

## Acknowledgements

Stellaverb was inspired by [Katsuhiro Chiba's Stellaverb](https://audiooo.com/stellaverb). This plugin is an independent implementation by Adrian Lam.

This work has made use of data from the European Space Agency (ESA) mission Gaia (https://www.cosmos.esa.int/gaia), processed by the Gaia Data Processing and Analysis Consortium (DPAC, https://www.cosmos.esa.int/web/gaia/dpac/consortium). Funding for the DPAC has been provided by national institutions, in particular the institutions participating in the Gaia Multilateral Agreement.

## Contact

Email: adrian1219@ymail.com

LinkedIn: https://www.linkedin.com/in/adrian-kt-lam
