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
    
    customLook.setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::black);
    
    customLook.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white.withAlpha(0.9f));
    customLook.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    
    customLook.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.9f));
    
    customLook.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    customLook.setColour(juce::TextButton::buttonOnColourId, juce::Colours::white.withAlpha(0.9f));
    customLook.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.9f));
    customLook.setColour(juce::TextButton::textColourOnId, juce::Colours::black.withAlpha(0.9f));
    
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
        auto minDistance = audioProcessor.parameters.getRawParameterValue("minDistance")->load();
        auto maxDistance = audioProcessor.parameters.getRawParameterValue("maxDistance")->load();
        auto minBrightness = audioProcessor.parameters.getRawParameterValue("minBrightness")->load();
        auto maxBrightness = audioProcessor.parameters.getRawParameterValue("maxBrightness")->load();
        auto generationMode = audioProcessor.parameters.getRawParameterValue("irGenerationMode")->load();
        
        DBG("length: " + juce::String(length)
            + ", minDistance: " + juce::String(minDistance)
            + ", maxDistance: " + juce::String(maxDistance));
        
        irGenerator.setLengthSeconds(length);
        irGenerator.setMinDistance(minDistance);
        irGenerator.setMaxDistance(maxDistance);
        irGenerator.setMinBrightness(minBrightness);
        irGenerator.setMaxBrightness(maxBrightness);
        irGenerator.setMode(generationMode);

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
    
    auto sliders = { &length, &minDistance, &maxDistance, &minBrightness, &maxBrightness, &predelay, &width, &dryWet, &gain, &lowFreq, &highFreq, &drive};
    auto labels  = { &lengthLabel, &minDistanceLabel, &maxDistanceLabel, &minBrightnessLabel, &maxBrightnessLabel, &predelayLabel, &widthLabel, &dryWetLabel, &gainLabel, &lowFreqLabel, &highFreqLabel, &driveLabel};
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

    lowFreqLabel.setText ("HP Cutoff", juce::dontSendNotification);
    lowFreqLabel.attachToComponent (&lowFreq, false);

    highFreqLabel.setText ("LP Cutoff", juce::dontSendNotification);
    highFreqLabel.attachToComponent (&highFreq, false);

    driveLabel.setText ("Drive", juce::dontSendNotification);
    driveLabel.attachToComponent (&drive, false);

    irGenerationModeLabel.setText ("IR Gen Mode", juce::dontSendNotification);
    irGenerationModeLabel.attachToComponent (&irGenerationMode, false);
    irGenerationModeLabel.setJustificationType (juce::Justification::centred);
    irGenerationModeLabel.setLookAndFeel(&customLook);
    addAndMakeVisible(irGenerationModeLabel);
    
    irGenerationMode.addItem("Acoustic", 1);
    irGenerationMode.addItem("Reverse", 2);
    irGenerationMode.addItem("Spectral", 3);
    
    irGenerationMode.setLookAndFeel(&customLook);
    addAndMakeVisible(irGenerationMode);

    
    lengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"length", length);
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
    
    irGenerationModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "irGenerationMode", irGenerationMode);

    length.onValueChange = [this]()
    {
        starDisplay.setCurrentLength(length.getValue() / 5.0f);
    };
    minDistance.onValueChange = [this]()
    {
        starDisplay.setMinDistance(minDistance.getValue());
    };
    maxDistance.onValueChange = [this]()
    {
        starDisplay.setMaxDistance(maxDistance.getValue());
    };
    minBrightness.onValueChange = [this]()
    {
        starDisplay.setMinBrightness(minBrightness.getValue());
    };
    maxBrightness.onValueChange = [this]()
    {
        starDisplay.setMaxBrightness(maxBrightness.getValue());
    };
    
    // Initialize GUI values on startup
    if (auto* lengthParam = audioProcessor.parameters.getRawParameterValue("length"))
        starDisplay.setCurrentLength(lengthParam->load() / 5.0f);
    if (auto* minDistanceParam = audioProcessor.parameters.getRawParameterValue("minDistance"))
        starDisplay.setMinDistance(minDistanceParam->load());
    if (auto* maxDistanceParam = audioProcessor.parameters.getRawParameterValue("maxDistance"))
        starDisplay.setMaxDistance(maxDistanceParam->load());
    if (auto* minBrightnessParam = audioProcessor.parameters.getRawParameterValue("minBrightness"))
        starDisplay.setMinBrightness(minBrightnessParam->load());
    if (auto* maxBrightnessParam = audioProcessor.parameters.getRawParameterValue("maxBrightness"))
        starDisplay.setMaxBrightness(maxBrightnessParam->load());;
    
    
    irGenerationMode.onChange = [this]()
    {
    };
    irGenerationMode.onChange();
    
    loadStarData();
    
    addAndMakeVisible(starDisplay);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(860, 520);
}

StellaverbAudioProcessorEditor::~StellaverbAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    irGenerationMode.setLookAndFeel(nullptr);
    irGenerationModeLabel.setLookAndFeel(nullptr);
    auto sliders = { &length, &minDistance, &maxDistance, &minBrightness, &maxBrightness, &predelay, &width, &dryWet, &gain, &lowFreq, &highFreq, &drive};
    for (auto* slider : sliders)
    {
        slider->setLookAndFeel(nullptr);
    }
    
    auto labels  = { &lengthLabel, &minDistanceLabel, &maxDistanceLabel, &minBrightnessLabel, &maxBrightnessLabel, &predelayLabel, &widthLabel, &dryWetLabel, &gainLabel, &lowFreqLabel, &highFreqLabel, &driveLabel};
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
    
    // Make entire left side with the stars black
    int starAreaSize = 360;
    int starPadding = 10;
    g.setColour(juce::Colours::black);
    // Draw the black box properly up to the boundary
    int rightAreaX = starPadding + starAreaSize; // move even more left
    g.fillRect(0, 0, rightAreaX, getHeight());

    int rightAreaWidth = getWidth() - rightAreaX;

    g.setColour (juce::Colours::white.withAlpha(0.9f));
    g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 24.0f, juce::Font::plain));
    g.drawText ("[Stellaverb]", rightAreaX, 20, rightAreaWidth, 40, juce::Justification::centred, true);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
    
    
    // waveformDisplay painting is handled by its own paint method
}

void StellaverbAudioProcessorEditor::resized()
{
    int starAreaSize = 360;
    int starPadding = 15;
    int starY = 15;
    starDisplay.setBounds(starPadding, starY, starAreaSize, starAreaSize);
    
    int rightAreaX = starPadding + starAreaSize + 5; // Match new left separation
    int rightAreaWidth = getWidth() - rightAreaX;

    int waveformY = starY + starAreaSize + 15;
    waveformDisplay.setBounds(starPadding, waveformY, starAreaSize, getHeight() - waveformY - 15);

    int sliderWidth = 88;
    int sliderHeight = 85;
    int padding = 6;
    int y = 92;
    
    // Center the sliders (top row - 5 options)
    int totalWidthTop = (sliderWidth * 5) + (padding * 4);
    int startX1 = rightAreaX + (rightAreaWidth - totalWidthTop) / 2;
    int x1 = startX1;
    
    length.setBounds(x1, y, sliderWidth, sliderHeight);
    x1 += sliderWidth + padding;
    minDistance.setBounds(x1, y, sliderWidth, sliderHeight);
    x1 += sliderWidth + padding;
    maxDistance.setBounds(x1, y, sliderWidth, sliderHeight);
    x1 += sliderWidth + padding;
    minBrightness.setBounds(x1, y, sliderWidth, sliderHeight);
    x1 += sliderWidth + padding;
    maxBrightness.setBounds(x1, y, sliderWidth, sliderHeight);
    
    // Generate IR button & Mode Combo
    int btnWidth = 120;
    int btnHeight = 35;
    int comboWidth = 120;
    
    int componentsWidth = btnWidth + comboWidth + padding;
    int btnX = rightAreaX + (rightAreaWidth - componentsWidth) / 2;
    
    int btnY = y + sliderHeight + 40; // Spacing below top row
    irGenerationMode.setBounds(btnX, btnY, comboWidth, btnHeight);
    generateButton.setBounds(btnX + comboWidth + padding, btnY, btnWidth, btnHeight);
    
    // Middle row (4 options)
    int row1Y = btnY + btnHeight + 40;
    int totalWidthRow1 = (sliderWidth * 4) + (padding * 3);
    int startXR1 = rightAreaX + (rightAreaWidth - totalWidthRow1) / 2;
    int x1_row1 = startXR1;
    
    predelay.setBounds(x1_row1, row1Y, sliderWidth, sliderHeight);
    x1_row1 += sliderWidth + padding;
    lowFreq.setBounds(x1_row1, row1Y, sliderWidth, sliderHeight);
    x1_row1 += sliderWidth + padding;
    highFreq.setBounds(x1_row1, row1Y, sliderWidth, sliderHeight);
    x1_row1 += sliderWidth + padding;
    drive.setBounds(x1_row1, row1Y, sliderWidth, sliderHeight);

    // Bottom row (3 options)
    int row2Y = row1Y + sliderHeight + 30;
    int totalWidthRow2 = (sliderWidth * 3) + (padding * 2);
    int startXR2 = rightAreaX + (rightAreaWidth - totalWidthRow2) / 2;
    int x1_row2 = startXR2;
    
    width.setBounds(x1_row2, row2Y, sliderWidth, sliderHeight);
    x1_row2 += sliderWidth + padding;
    dryWet.setBounds(x1_row2, row2Y, sliderWidth, sliderHeight);
    x1_row2 += sliderWidth + padding;
    gain.setBounds(x1_row2, row2Y, sliderWidth, sliderHeight);
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

    float arcThickness = 1.5f;
    float startAngle = rotaryStartAngle;
    float endAngle = angle;
    
    // faint full arc background
    juce::Path fullArc;
    fullArc.addCentredArc(centre.x, centre.y,
                          radius * 0.85f, radius * 0.85f,
                          0.0f,
                          rotaryStartAngle,
                          rotaryEndAngle,
                          true);

    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.strokePath(fullArc, juce::PathStrokeType(arcThickness));

    juce::Path pointer;
    float pointerLength = radius * 0.65f;
    float pointerThickness = 1.5f;
    
    juce::Path valueArc;

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
