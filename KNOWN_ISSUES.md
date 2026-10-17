# Known issues

These notes apply to Stellaverb **0.3.0**.

## Offline export with IR parameter automation

Offline exports can sound different from realtime playback when automating parameters that regenerate the impulse response:

- IR length
- Minimum and maximum distance
- Minimum and maximum brightness
- IR generation mode

IR generation runs in the background, and its updates are not synchronised with offline rendering. IR changes are not sample-accurate.

**Workaround:** use fixed IR generation settings for offline exports, allow the IR to finish updating before rendering, and check the exported audio. If you need these parameters to change during a track, capture realtime playback and check the result.

## Temporary volume drop when changing IR parameters

The reverb output can briefly drop in volume when a newly generated impulse response is loaded into the convolution engine. 

Affected parameters include IR length, minimum/maximum distance, minimum/maximum brightness, and generation mode.

**Workaround:** keep these parameters fixed during passages where a consistent reverb level is important, or make changes between phrases.

## Star display shows a subset of the dataset

The GUI displays a fixed subset of up to 1,000 stars. The IR uses all stars matching the selection filters, so the display does not show every contributing star. A narrow selection can show few or no dots even when stars outside the displayed subset contribute to the IR.

## macOS installation and compatibility

- The current macOS builds support **Apple Silicon only**, in **VST3 and AU** formats. Intel Macs and DAWs running under Rosetta are not supported by these builds.
- The builds target **macOS 11 or later**, but compatibility across all supported macOS versions has not been verified.
- The builds are **ad-hoc signed and not notarised**. macOS or the DAW may block downloaded copies. Installation and loading of the release ZIP on a separate Mac remain to be verified.
- Windows and AAX builds are not included in this beta.

## Reporting an issue

Open an issue on [GitHub](https://github.com/adrian-1219/Stellaverb/issues) or email **adrian1219@ymail.com**. Include:

- Stellaverb version and plugin format (VST3 or AU)
- OS and version
- DAW name and version
- Sample rate and audio buffer size
- Steps to reproduce the issue and the expected result
- Relevant preset/settings, screenshots, or the exact error message

For audio problems, mention whether they occur during realtime playback, offline export, or both.
