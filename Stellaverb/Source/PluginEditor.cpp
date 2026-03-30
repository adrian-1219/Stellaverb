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
    addAndMakeVisible(generateButton);
    generateButton.onClick = [this]()
    {
        auto length = audioProcessor.parameters.getRawParameterValue("length")->load();
        auto decayTime = audioProcessor.parameters.getRawParameterValue("decayTime")->load();
        auto numReflections = audioProcessor.parameters.getRawParameterValue("numReflections")->load();
        
        DBG("length: " + juce::String(length)
            + ", decayTime: " + juce::String(decayTime)
            + ", numReflections: " + juce::String(numReflections));
        
        irGenerator.setLengthSeconds(length);
        irGenerator.setDecay(decayTime);
        irGenerator.setNumReflections(numReflections);
        irGenerator.setLowpassFrequency(5000.0f);

        irGenerator.generate();

        juce::File file = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Stellaverb")
            .getChildFile("generated_ir.wav");

        irGenerator.saveToWav(file);
        
        audioProcessor.prepareToPlay(audioProcessor.getSampleRate(), audioProcessor.getBlockSize());
        
        DBG("Successfully generated IR");
    };
    
    auto sliders = { &length, &decayTime, &numReflections};
    auto labels  = { &lengthLabel, &decayTimeLabel, &numReflectionsLabel};
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
    
    numReflectionsLabel.setText ("Num Reflections", juce::dontSendNotification);
    numReflectionsLabel.attachToComponent (&numReflections, false);
    
    lengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"length", length);
    decayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "decayTime", decayTime);
    numReflectionsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "numReflections", numReflections);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
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
}

void StellaverbAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    int sliderWidth = 100;
    int sliderHeight = 100;
    int padding = 20;
    int y = 80;
    int x = padding;
    
    length.setBounds(x, y, sliderWidth, sliderHeight);
    x += sliderWidth + padding;
    decayTime.setBounds(x, y, sliderWidth, sliderHeight);
    x += sliderWidth + padding;
    numReflections.setBounds(x, y, sliderWidth, sliderHeight);
}
