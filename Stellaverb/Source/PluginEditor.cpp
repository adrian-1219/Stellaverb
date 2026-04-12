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
        auto minBrightness = audioProcessor.parameters.getRawParameterValue("minBrightness")->load();
        auto maxBrightness = audioProcessor.parameters.getRawParameterValue("maxBrightness")->load();
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
        irGenerator.setMinBrightness(minBrightness);
        irGenerator.setMaxBrightness(maxBrightness);

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
    
    auto sliders = { &length, &decayTime, &minDistance, &maxDistance, &minBrightness, &maxBrightness, &predelay, &width, &dryWet, &gain, &lowPassFreq, &highPassFreq, &colour};
    auto labels  = { &lengthLabel, &decayTimeLabel, &minDistanceLabel, &maxDistanceLabel, &minBrightnessLabel, &maxBrightnessLabel, &predelayLabel, &widthLabel, &dryWetLabel, &gainLabel, &lowPassFreqLabel, &highPassFreqLabel, &colourLabel};
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
    
    minBrightnessLabel.setText ("Min Brightness", juce::dontSendNotification);
    minBrightnessLabel.attachToComponent (&minBrightness, false);
    
    maxBrightnessLabel.setText ("Max Brightness", juce::dontSendNotification);
    maxBrightnessLabel.attachToComponent (&maxBrightness, false);
    
    predelayLabel.setText ("Pre-delay", juce::dontSendNotification);
    predelayLabel.attachToComponent (&predelay, false);
    
    widthLabel.setText ("Width", juce::dontSendNotification);
    widthLabel.attachToComponent (&width, false);
    
    dryWetLabel.setText ("Dry/Wet", juce::dontSendNotification);
    dryWetLabel.attachToComponent (&dryWet, false);

    gainLabel.setText ("Gain", juce::dontSendNotification);
    gainLabel.attachToComponent (&gain, false);

    lowPassFreqLabel.setText ("Low Pass", juce::dontSendNotification);
    lowPassFreqLabel.attachToComponent (&lowPassFreq, false);

    highPassFreqLabel.setText ("High Pass", juce::dontSendNotification);
    highPassFreqLabel.attachToComponent (&highPassFreq, false);

    colourLabel.setText ("Colour", juce::dontSendNotification);
    colourLabel.attachToComponent (&colour, false);
    
    lengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"length", length);
    decayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "decayTime", decayTime);
    minDistanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "minDistance", minDistance);
    maxDistanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "maxDistance", maxDistance);
    minBrightnessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "minBrightness", minBrightness);
    maxBrightnessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "maxBrightness", maxBrightness);
    predelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "preDelay", predelay);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "width", width);
    dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "dryWet", dryWet);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gain", gain);
    lowPassFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lowPassFreq", lowPassFreq);
    highPassFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "highPassFreq", highPassFreq);
    colourAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "colour", colour);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (900, 550);
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
    
    // Center the sliders (top row - 6 options)
    int totalWidthTop = (sliderWidth * 6) + (padding * 5);
    int startX1 = (getWidth() - totalWidthTop) / 2;
    int x1 = startX1;
    
    length.setBounds(x1, y, sliderWidth, sliderHeight);
    x1 += sliderWidth + padding;
    decayTime.setBounds(x1, y, sliderWidth, sliderHeight);
    x1 += sliderWidth + padding;
    minDistance.setBounds(x1, y, sliderWidth, sliderHeight);
    x1 += sliderWidth + padding;
    maxDistance.setBounds(x1, y, sliderWidth, sliderHeight);
    x1 += sliderWidth + padding;
    minBrightness.setBounds(x1, y, sliderWidth, sliderHeight);
    x1 += sliderWidth + padding;
    maxBrightness.setBounds(x1, y, sliderWidth, sliderHeight);
    
    // Center the sliders (bottom row - 7 options)
    int bottomRowY = getHeight() - sliderHeight - 20;
    int totalWidthBottom = (sliderWidth * 7) + (padding * 6);
    int startX2 = (getWidth() - totalWidthBottom) / 2;
    int x2 = startX2;
    
    predelay.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    width.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    lowPassFreq.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    highPassFreq.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    colour.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    dryWet.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    gain.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);

    // Place waveform display below sliders and above bottom row
    int waveformY = y + sliderHeight + padding + 20;
    waveformDisplay.setBounds(20, waveformY, getWidth() - 40, bottomRowY - waveformY - 20);
}
