/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "IRGenerator.h"

//==============================================================================
/**
*/
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;
};

class StellaverbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    StellaverbAudioProcessorEditor (StellaverbAudioProcessor&);
    ~StellaverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class WaveformDisplay : public juce::Component
    {
    public:
        WaveformDisplay(juce::AudioThumbnail& thumb) : thumbnail(thumb) {}
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colours::black);
            g.setColour(juce::Colours::white);
            if (thumbnail.getNumChannels() > 0)
                thumbnail.drawChannels(g, getLocalBounds(), 0.0, thumbnail.getTotalLength(), 1.0f);
            else
                g.drawText("No waveform loaded", getLocalBounds(), juce::Justification::centred);
        }
        void resized() override {}
    private:
        juce::AudioThumbnail& thumbnail;
    };

    juce::TextButton generateButton { "Generate IR" };
    IRGenerator irGenerator = IRGenerator(44100.0f, 2);
    
    StellaverbAudioProcessor& audioProcessor;
    juce::Slider length, decayTime, minDistance, maxDistance, minBrightness, maxBrightness, predelay, width, dryWet, gain, lowFreq, highFreq, drive;
    juce::Label lengthLabel, decayTimeLabel, minDistanceLabel, maxDistanceLabel, minBrightnessLabel, maxBrightnessLabel, predelayLabel, widthLabel, dryWetLabel, gainLabel, lowFreqLabel, highFreqLabel, driveLabel;

    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lengthAttachment, decayTimeAttachment, minDistanceAttachment, maxDistanceAttachment, minBrightnessAttachment, maxBrightnessAttachment, predelayAttachment, widthAttachment, dryWetAttachment, gainAttachment, lowFreqAttachment, highFreqAttachment, driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distanceScaleModeAttachment;
    
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache { 10 };
    juce::AudioThumbnail thumbnail { 512, formatManager, thumbnailCache };
    WaveformDisplay waveformDisplay { thumbnail };
    
    CustomLookAndFeel customLook;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StellaverbAudioProcessorEditor)
};


