
# Stellaverb

Inspired by [Katsuhiro Chiba's Max MSP patch](https://audiooo.com/stellaverb), Stellaverb is a convolution reverb plugin that transforms astronomical data into unique reverberation textures. Drawing from the ESA Gaia Star Catalogue, it imagines what a reverb in space would sound like, while also enabling more abstract, creative reverbs shaped by stellar properties such as distance, brightness, and position.

This project was developed as the final assignment for the ECS7012 Music and Audio Programming module at Queen Mary University of London.

## Demo Video

Here is a short demo video of what the plugin sounds like.

https://github.com/user-attachments/assets/cb92aee9-d773-4f54-8f14-84a707983d79

## Install

Download the ZIP for your platform from the [Releases page](https://github.com/adrian-1219/Stellaverb/releases/). Extract the ZIP file and follow the instructions in INSTALL.txt. 

Below are the available builds for Stellaverb v0.3.0.

| OS | Architecture | Format |
| -- | -- | -- |
| Windows | x64 | VST3 |
| macOS | Apple Silicon (arm64) | VST3, AU |

## Features

![Screenshot of UI](./assets/ui.png)

The core of the plugin is its ability to generate an impulse response (IR) using the star data based on the user's settings, then convolving the input signal with the generated IR. Each star is mapped to a sample in the IR based on its distance, position and brightness.

- **Reverb generated from star data:** transforms distance, brightness, and position from the ESA Gaia star catalogue into stereo impulse responses.
- **Three generation modes:**
  - **Acoustic:** closer stars contribute more strongly, creating a decaying reverb.
  - **Reverse:** more distant stars contribute more strongly, creating a rising reverb.
  - **Sustain:** brightness determines each star's contribution, creating sustained textures.
- **Adjustable IR generation:** control the impulse response length and filter stars by distance and brightness.
- **Interactive star visualisation:** displays a subset of the stars and responds to your selection settings.
- **Sound-shaping controls:** pre-delay, highpass and lowpass filters, drive, stereo width, dry/wet mix, and output gain.
- **Factory and user presets:** explore included sounds and save your own.
- **WAV export:** export generated impulse responses for use in other convolution tools.
- **Open source:** available under the GNU AGPLv3 licence.

## Build Instructions

Use JUCE 8.0.12, including Projucer.

1. Download JUCE 8.0.12.
2. Open Stellaverb/Stellaverb.jucer in Projucer.
3. Set Projucer's global JUCE modules path to the downloaded JUCE modules folder.
4. Select the exporter for your platform. (Xcode for macOS, Visual Studio for Windows)
5. Click Save and Open in IDE to generate and open the platform-specific project.

macOS: Build the VST3 and/or AU targets using the Release configuration.

Windows: Select Release and x64 in Visual Studio, then build the VST3 project. AU is only available on macOS.

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
