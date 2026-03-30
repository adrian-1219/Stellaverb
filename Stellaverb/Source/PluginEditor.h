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
class StellaverbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    StellaverbAudioProcessorEditor (StellaverbAudioProcessor&);
    ~StellaverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.

    juce::TextButton generateButton { "Generate IR" };
    IRGenerator irGenerator = IRGenerator(44100.0f, 2);
    
    StellaverbAudioProcessor& audioProcessor;
    juce::Slider length, decayTime, numReflections;
    juce::Label lengthLabel, decayTimeLabel, numReflectionsLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lengthAttachment,decayTimeAttachment, numReflectionsAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StellaverbAudioProcessorEditor)
};
