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
    
    customLook.setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xfffff8e1));
    
    customLook.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black.withAlpha(0.8f));
    customLook.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    
    customLook.setColour(juce::Label::textColourId, juce::Colours::black.withAlpha(0.9f));
    
    customLook.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1f1f1f));
    customLook.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    customLook.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    customLook.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    
    setLookAndFeel(&customLook);

    juce::File file = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Stellaverb")
        .getChildFile("generated_ir.wav");
    
    // Load the generated IR file into the thumbnail
    thumbnail.setSource(nullptr);
    thumbnail.clear();
    
    thumbnail.setSource(new juce::FileInputSource(file));
    
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
        auto distanceScaleMode = audioProcessor.parameters.getRawParameterValue("scaleMode")->load();
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

        irGenerator.generate(distanceScaleMode);
        
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
    
    auto sliders = { &length, &decayTime, &minDistance, &maxDistance, &minBrightness, &maxBrightness, &predelay, &width, &dryWet, &gain, &lowFreq, &highFreq, &drive};
    auto labels  = { &lengthLabel, &decayTimeLabel, &minDistanceLabel, &maxDistanceLabel, &minBrightnessLabel, &maxBrightnessLabel, &predelayLabel, &widthLabel, &dryWetLabel, &gainLabel, &lowFreqLabel, &highFreqLabel, &driveLabel};
    for (auto* slider : sliders)
    {
        addAndMakeVisible (slider);
        slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
        slider->setLookAndFeel(&customLook);
    }
    for (auto* label : labels)
    {
        addAndMakeVisible (label);
        label->setJustificationType (juce::Justification::centred);
        label->setBorderSize(juce::BorderSize<int>(0));
        label->setLookAndFeel(&customLook);
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

    lowFreqLabel.setText ("Low Pass", juce::dontSendNotification);
    lowFreqLabel.attachToComponent (&lowFreq, false);

    highFreqLabel.setText ("High Pass", juce::dontSendNotification);
    highFreqLabel.attachToComponent (&highFreq, false);

    driveLabel.setText ("Drive", juce::dontSendNotification);
    driveLabel.attachToComponent (&drive, false);

    
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
    lowFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lowFreq", lowFreq);
    highFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "highFreq", highFreq);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "drive", drive);

    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (900, 550);
}

StellaverbAudioProcessorEditor::~StellaverbAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    auto sliders = { &length, &decayTime, &minDistance, &maxDistance, &minBrightness, &maxBrightness, &predelay, &width, &dryWet, &gain, &lowFreq, &highFreq, &drive};
    for (auto* slider : sliders)
    {
        slider->setLookAndFeel(nullptr);
    }
    
    auto labels  = { &lengthLabel, &decayTimeLabel, &minDistanceLabel, &maxDistanceLabel, &minBrightnessLabel, &maxBrightnessLabel, &predelayLabel, &widthLabel, &dryWetLabel, &gainLabel, &lowFreqLabel, &highFreqLabel, &driveLabel};
    for (auto* label : labels)
    {
        label->setLookAndFeel(nullptr);
    }
    
}

//==============================================================================
void StellaverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::orange.withAlpha(0.9f));
    g.setFont (juce::FontOptions (36.0f, juce::Font::bold));
    g.drawText ("Stellaverb", 0, 20, getWidth(), 40, juce::Justification::centred, true);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    
    
    // waveformDisplay painting is handled by its own paint method
}

void StellaverbAudioProcessorEditor::resized()
{
    int sliderWidth = 100;
    int sliderHeight = 100;
    int padding = 20;
    int y = 90;
    
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
    
    // Generate IR button
    int btnWidth = 200;
    int btnHeight = 35;
    int btnY = y + sliderHeight + 10;
    generateButton.setBounds((getWidth() - btnWidth) / 2, btnY, btnWidth, btnHeight);
    
    // Center the sliders (bottom row - 7 options)
    int bottomRowY = getHeight() - sliderHeight - 20;
    int totalWidthBottom = (sliderWidth * 7) + (padding * 6);
    int startX2 = (getWidth() - totalWidthBottom) / 2;
    int x2 = startX2;
    
    predelay.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    lowFreq.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    highFreq.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    drive.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    width.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    dryWet.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);
    x2 += sliderWidth + padding;
    gain.setBounds(x2, bottomRowY, sliderWidth, sliderHeight);

    // Place waveform display below sliders and above bottom row
    int waveformY = btnY + btnHeight + padding;
    waveformDisplay.setBounds(20, waveformY, getWidth() - 40, bottomRowY - waveformY - 40);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPos,
                                         float rotaryStartAngle,
                                         float rotaryEndAngle,
                                         juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);

    auto centre = bounds.getCentre();
    float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 6.0f;

    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour(juce::Colours::transparentBlack);
    g.fillEllipse(bounds);
    juce::Colour baseColour = juce::Colours::black.withAlpha(0.9f);

    g.setColour(baseColour);
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    // subtle inner shadow
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

    juce::Path valueArc;

    float arcThickness = 3.5f;
    float startAngle = rotaryStartAngle;
    float endAngle = angle;

    valueArc.addCentredArc(centre.x, centre.y,
                           radius * 0.85f, radius * 0.85f,
                           0.0f,
                           startAngle,
                           endAngle,
                           true);

    g.setColour(juce::Colours::orange.withAlpha(0.9f));
    g.strokePath(valueArc, juce::PathStrokeType(arcThickness,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    // faint full arc background
    juce::Path fullArc;
    fullArc.addCentredArc(centre.x, centre.y,
                          radius * 0.85f, radius * 0.85f,
                          0.0f,
                          rotaryStartAngle,
                          rotaryEndAngle,
                          true);

    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.strokePath(fullArc, juce::PathStrokeType(arcThickness));

    juce::Path pointer;
    float pointerLength = radius * 0.65f;
    float pointerThickness = 3.0f;

    pointer.addRoundedRectangle(-pointerThickness * 0.5f,
                                -pointerLength,
                                pointerThickness,
                                pointerLength,
                                2.0f);

    g.setColour(juce::Colours::white.withAlpha(0.9f));

    g.fillPath(pointer,
               juce::AffineTransform::rotation(angle)
                   .translated(centre.x, centre.y));

}
