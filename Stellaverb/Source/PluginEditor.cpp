/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
StellaverbAudioProcessorEditor::StellaverbAudioProcessorEditor (StellaverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    formatManager.registerBasicFormats();

    addAndMakeVisible(waveformDisplay);
    
    addAndMakeVisible(generateButton);
    generateButton.onClick = [this]()
    {
        audioProcessor.irReady = false;
        
        auto length = audioProcessor.parameters.getRawParameterValue("length")->load();
        auto decayTime = audioProcessor.parameters.getRawParameterValue("decayTime")->load();
        auto minDistance = audioProcessor.parameters.getRawParameterValue("minDistance")->load();
        auto maxDistance = audioProcessor.parameters.getRawParameterValue("maxDistance")->load();
        minDistance = minDistance / 7.0f;
        maxDistance = maxDistance / 7.0f; // scale from parsecs to 0 to 1
        
        DBG("length: " + juce::String(length)
            + ", decayTime: " + juce::String(decayTime)
            + ", minDistance: " + juce::String(minDistance)
            + ", maxDistance: " + juce::String(maxDistance));
        
        irGenerator.setLengthSeconds(length);
        irGenerator.setDecay(decayTime);
        irGenerator.setMinDistance(minDistance);
        irGenerator.setMaxDistance(maxDistance);

        irGenerator.generate();

        juce::File file = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Stellaverb")
            .getChildFile("generated_ir.wav");

        irGenerator.saveToWav(file);
        
        // Load the generated IR file into the thumbnail
        thumbnail.setSource(nullptr);
        thumbnail.clear();

        thumbnail.setSource(new juce::FileInputSource(file));
        
        DBG(thumbnail.getTotalLength());
        
        waveformDisplay.repaint();
        
        DBG("Successfully generated IR");
        
        audioProcessor.prepareToPlay(audioProcessor.getSampleRate(), audioProcessor.getBlockSize());
    };
    
    auto sliders = { &length, &decayTime, &minDistance, &maxDistance};
    auto labels  = { &lengthLabel, &decayTimeLabel, &minDistanceLabel, &maxDistanceLabel};
    for (auto* slider : sliders)
    {
        addAndMakeVisible (slider);
        slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    }
    for (auto* label : labels)
    {
        addAndMakeVisible (label);
        label->setJustificationType (juce::Justification::centred);
        label->setBorderSize(juce::BorderSize<int>(0));
    }
    
    lengthLabel.setText ("Length", juce::dontSendNotification);
    lengthLabel.attachToComponent (&length, false);
    
    decayTimeLabel.setText ("Decay Time", juce::dontSendNotification);
    decayTimeLabel.attachToComponent (&decayTime, false);
    
    minDistanceLabel.setText ("Min Distance", juce::dontSendNotification);
    minDistanceLabel.attachToComponent (&minDistance, false);
    
    maxDistanceLabel.setText ("Max Distance", juce::dontSendNotification);
    maxDistanceLabel.attachToComponent (&maxDistance, false);
    
    lengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"length", length);
    decayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "decayTime", decayTime);
    minDistanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "minDistance", minDistance);
    maxDistanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "maxDistance", maxDistance);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
}

StellaverbAudioProcessorEditor::~StellaverbAudioProcessorEditor()
{
}

//==============================================================================
void StellaverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    
    generateButton.setBounds(20, 20, 150, 40);
    // waveformDisplay painting is handled by its own paint method
}

void StellaverbAudioProcessorEditor::resized()
{
    int sliderWidth = 100;
    int sliderHeight = 100;
    int padding = 20;
    int y = 80;
    
    // Center the sliders
    int totalWidth = (sliderWidth * 4) + (padding * 3);
    int startX = (getWidth() - totalWidth) / 2;
    int x = startX;
    
    length.setBounds(x, y, sliderWidth, sliderHeight);
    x += sliderWidth + padding;
    decayTime.setBounds(x, y, sliderWidth, sliderHeight);
    x += sliderWidth + padding;
    minDistance.setBounds(x, y, sliderWidth, sliderHeight);
    x += sliderWidth + padding;
    maxDistance.setBounds(x, y, sliderWidth, sliderHeight);
    
    // Place waveform display below sliders
    int waveformY = y + sliderHeight + padding + 20;
    waveformDisplay.setBounds(20, waveformY, getWidth() - 40, getHeight() - waveformY - 20);
}
