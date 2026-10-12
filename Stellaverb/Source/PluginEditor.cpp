/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

class ModeHelpItem : public juce::PopupMenu::CustomComponent
{
public:
    ModeHelpItem(const juce::String& name,
                 const juce::String& description)
        : title(name), helpText(description)
    {
    }

    void getIdealSize(int& width, int& height) override
    {
        width = 180;
        height = 30;
    }

    void paint(juce::Graphics& g) override
    {
        const auto* item = getItem();

        getLookAndFeel().drawPopupMenuItem(
            g, getLocalBounds(),
            false, true, isItemHighlighted(),
            item != nullptr && item->isTicked,
            false, title, {}, nullptr, nullptr);
    }

    juce::String helpText;

private:
    juce::String title;
};


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

    customLook.setColour(juce::ComboBox::backgroundColourId, juce::Colours::black);
    customLook.setColour(juce::ComboBox::textColourId, juce::Colours::white.withAlpha(0.9f));
    customLook.setColour(juce::ComboBox::outlineColourId, juce::Colours::white.withAlpha(0.5f));
    customLook.setColour(juce::ComboBox::arrowColourId, juce::Colours::white.withAlpha(0.9f));
    
    customLook.setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black);
    customLook.setColour(juce::PopupMenu::textColourId, juce::Colours::white.withAlpha(0.9f));
    customLook.setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::white.withAlpha(0.2f));
    customLook.setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    
    setLookAndFeel(&customLook);
    
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(exportButton);
    
    exportButton.onClick = [this]()
    {
        exportIR();
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
    
    auto* modeMenu = irGenerationMode.getRootMenu();

    modeMenu->addCustomItem(
        1,
        std::make_unique<ModeHelpItem>(
            "Acoustic",
            "Acoustic: Closer stars contribute more strongly to the reverb."),
        nullptr, "Acoustic");

    modeMenu->addCustomItem(
        2,
        std::make_unique<ModeHelpItem>(
            "Reverse",
            "Reverse: More distant stars contribute more strongly to the reverb."),
        nullptr, "Reverse");

    modeMenu->addCustomItem(
        3,
        std::make_unique<ModeHelpItem>(
            "Sustain",
            "Sustain: Each star's contribution depends on its brightness."),
        nullptr, "Sustain");
    
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

    std::vector<StarGUI> subset;

    const size_t count = std::min(stars.size(), size_t { 1000 });
    subset.reserve(count);

    for (size_t i = 0; i < count; ++i)
    {
        const size_t index = i * stars.size() / count;
        subset.push_back(stars[index]);
    }

    stars = std::move(subset);
    
    addAndMakeVisible(starDisplay);
    addAndMakeVisible(helpLabel);

    helpLabel.setJustificationType(juce::Justification::centredLeft);
    helpLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::white.withAlpha(0.75f));

    helpLabel.setFont(juce::FontOptions(13.0f));
    helpLabel.setInterceptsMouseClicks(false, false);
    helpLabel.setText(
        "Hover over a control for help.",
        juce::dontSendNotification);

    length.setTooltip("Length: Sets the duration of the generated impulse response.");
    minDistance.setTooltip("Min distance: Excludes stars nearer than this relative distance.");
    maxDistance.setTooltip("Max distance: Excludes stars farther than this relative distance.");
    minBrightness.setTooltip("Min brightness: Excludes stars below this relative brightness.");
    maxBrightness.setTooltip("Max brightness: Excludes stars above this relative brightness.");

    predelay.setTooltip("Pre-delay: Adds a delay before the reverb begins.");
    lowFreq.setTooltip("HP cutoff: Removes low frequencies from the reverb.");
    highFreq.setTooltip("LP cutoff: Removes high frequencies from the reverb.");
    drive.setTooltip("Drive: Adds saturation to the reverb.");
    width.setTooltip("Width: Adjusts the stereo spread of the reverb.");
    dryWet.setTooltip("Dry/Wet: Balances the original signal and the reverb.");
    gain.setTooltip("Gain: Adjusts the final output level.");

    irGenerationMode.setTooltip(
        "IR Gen Mode: Chooses the mapping used when generating the impulse response.");

    exportButton.setTooltip(
        "Export IR: Saves the latest generated impulse response as a WAV file.");
    
    lengthLabel.setTooltip(length.getTooltip());
    minDistanceLabel.setTooltip(minDistance.getTooltip());
    maxDistanceLabel.setTooltip(maxDistance.getTooltip());
    minBrightnessLabel.setTooltip(minBrightness.getTooltip());
    maxBrightnessLabel.setTooltip(maxBrightness.getTooltip());
    predelayLabel.setTooltip(predelay.getTooltip());
    lowFreqLabel.setTooltip(lowFreq.getTooltip());
    highFreqLabel.setTooltip(highFreq.getTooltip());
    driveLabel.setTooltip(drive.getTooltip());
    widthLabel.setTooltip(width.getTooltip());
    dryWetLabel.setTooltip(dryWet.getTooltip());
    gainLabel.setTooltip(gain.getTooltip());
    irGenerationModeLabel.setTooltip(irGenerationMode.getTooltip());
    
    addAndMakeVisible(presetButton);
    presetButton.setTooltip("Load or save a preset.");
    presetButton.onClick = [this]() { showPresetMenu(); };
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.attachToComponent(&presetButton, false);
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setTooltip("Load or save a preset.");
    addAndMakeVisible(presetLabel);
    updatePresetButton();

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(860, 540);
    
    refreshWaveformFromCache();
    startTimer(100);
}

StellaverbAudioProcessorEditor::~StellaverbAudioProcessorEditor()
{
    stopTimer();
    exportChooser.reset();
    presetSaveChooser.reset();
    
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

bool StellaverbAudioProcessorEditor::writeIRToWav(
    const juce::File& destination,
    const juce::AudioBuffer<float>& buffer)
{
    // Write a temporary file first, then replace the destination
    // only after writing succeeds.
    juce::TemporaryFile temporaryFile(destination);

    auto stream = temporaryFile.getFile().createOutputStream();

    if (stream == nullptr || !stream->openedOk())
        return false;

    juce::WavAudioFormat format;

    std::unique_ptr<juce::AudioFormatWriter> writer(
        format.createWriterFor(stream.get(), 44100.0, static_cast<unsigned int>(buffer.getNumChannels()),
            24, {}, 0));

    if (writer == nullptr)
        return false;

    // The writer now owns the stream.
    stream.release();
    const bool written = writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    const bool flushed = writer->flush();

    // Close the writer before moving the temporary file.
    writer.reset();

    if (!written || !flushed)
        return false;

    return temporaryFile.overwriteTargetFileWithTemporary();
}

void StellaverbAudioProcessorEditor::exportIR()
{
    // Keep this snapshot alive until the dialog finishes.
    auto snapshot =
        std::make_shared<juce::AudioBuffer<float>>();

    juce::uint64 version = 0;

    if (!audioProcessor.copyIRForDisplay(*snapshot, version)
        || snapshot->getNumSamples() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "No IR available",
            "Wait for an impulse response to finish generating.");

        return;
    }

    exportChooser = std::make_unique<juce::FileChooser>(
        "Export impulse response",
        juce::File::getSpecialLocation(
            juce::File::userDocumentsDirectory)
            .getChildFile("Stellaverb IR.wav"),
        "*.wav");

    exportButton.setEnabled(false);

    juce::Component::SafePointer<
        StellaverbAudioProcessorEditor> safeThis(this);

    const int flags =
        juce::FileBrowserComponent::saveMode
        | juce::FileBrowserComponent::canSelectFiles
        | juce::FileBrowserComponent::warnAboutOverwriting;

    exportChooser->launchAsync(
        flags,
        [safeThis, snapshot](const juce::FileChooser& chooser)
        {
            if (safeThis == nullptr)
                return;

            safeThis->exportButton.setEnabled(true);

            const auto destination = chooser.getResult();

            // The user cancelled.
            if (destination == juce::File())
                return;

            // Keep the exact filename chosen in the dialog,
            // including its overwrite confirmation.
            if (!destination.hasFileExtension("wav"))
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Use a WAV filename",
                    "Please export again with a filename ending in .wav.");

                return;
            }

            if (!writeIRToWav(destination, *snapshot))
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Export failed",
                    "Could not write the WAV file. "
                    "Check the destination and available disk space.");
            }
        });
}

void StellaverbAudioProcessorEditor::refreshWaveformFromCache()
{
    juce::AudioBuffer<float> buffer;

    if (!audioProcessor.copyIRForDisplay(
            buffer, lastDisplayedIRVersion))
        return;

    thumbnail.reset(
        buffer.getNumChannels(),
        44100.0,
        buffer.getNumSamples());

    thumbnail.addBlock(
        0,
        buffer,
        0,
        buffer.getNumSamples());

    waveformDisplay.repaint();
}

void StellaverbAudioProcessorEditor::timerCallback()
{
    refreshWaveformFromCache();
    updateHelpText();
    updatePresetButton();
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
    
    juce::AttributedString credit;
    credit.setJustification(juce::Justification::centredRight);

    credit.append("Stellaverb by ", juce::Colours::white.withAlpha(0.75f));
    credit.append("Adrian Lam", juce::Colours::orange);
    
    credit.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain)));

    credit.draw(g, juce::Rectangle<float>(
        getWidth() - 235.0f,
        getHeight() - 32.0f,
        220.0f,
        32.0f));
    
    
    // waveformDisplay painting is handled by its own paint method
}

void StellaverbAudioProcessorEditor::resized()
{
    constexpr int helpHeight = 32;

    const int footerY = getHeight() - helpHeight;
    const int contentHeight = footerY;

    helpLabel.setBounds(
        15, footerY,
        juce::jmax(0, getWidth() - 265),
        helpHeight);

    int starAreaSize = 360;
    int starPadding = 15;
    int starY = 15;
    starDisplay.setBounds(starPadding, starY, starAreaSize, starAreaSize);
    
    int rightAreaX = starPadding + starAreaSize + 5; // Match new left separation
    int rightAreaWidth = getWidth() - rightAreaX;

    int waveformY = starY + starAreaSize + 15;
    waveformDisplay.setBounds(starPadding, waveformY, starAreaSize,  juce::jmax(0, contentHeight - waveformY - 15));

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
    
    // IR mode, presets and IR export
    int btnWidth = 120;
    int btnHeight = 35;
    int comboWidth = 120;
    
    int componentsWidth = comboWidth + 2 * btnWidth + 2 * padding;
    int btnX = rightAreaX + (rightAreaWidth - componentsWidth) / 2;
    
    int btnY = y + sliderHeight + 40; // Spacing below top row
    irGenerationMode.setBounds(btnX, btnY, comboWidth, btnHeight);
    presetButton.setBounds(btnX + comboWidth + padding, btnY, btnWidth, btnHeight);
    exportButton.setBounds(btnX + comboWidth + btnWidth + 2 * padding,
                           btnY, btnWidth, btnHeight);
    
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

void StellaverbAudioProcessorEditor::updateHelpText()
{
    if (irGenerationMode.isPopupActive())
    {
        juce::PopupMenu::MenuItemIterator iterator(
            *irGenerationMode.getRootMenu());

        while (iterator.next())
        {
            auto* item = dynamic_cast<ModeHelpItem*>(
                iterator.getItem().customComponent.get());

            if (item != nullptr && item->isItemHighlighted())
            {
                if (helpLabel.getText() != item->helpText)
                    helpLabel.setText(
                        item->helpText, juce::dontSendNotification);

                return;
            }
        }
    }
    
    juce::String text = "Hover over a control for help.";

    auto* component = juce::Desktop::getInstance()
        .getMainMouseSource()
        .getComponentUnderMouse();

    // Only inspect controls belonging to this editor.
    if (component != nullptr && isParentOf(component))
    {
        while (component != nullptr && component != this)
        {
            if (auto* client =
                    dynamic_cast<juce::TooltipClient*>(component))
            {
                const auto tooltip = client->getTooltip();

                if (tooltip.isNotEmpty())
                {
                    text = tooltip;
                    break;
                }
            }

            component = component->getParentComponent();
        }
    }

    if (helpLabel.getText() != text)
        helpLabel.setText(text, juce::dontSendNotification);
}

void StellaverbAudioProcessorEditor::updatePresetButton()
{
    const auto name = audioProcessor.presetManager.getCurrentPresetName();
    if (presetButton.getButtonText() != name)
        presetButton.setButtonText(name);
    presetButton.setTooltip("Preset: " + name + ". Click to load or save a preset.");
}

void StellaverbAudioProcessorEditor::showPresetMenu()
{
    // Re-read each time so presets saved in another instance also appear.
    const auto names = audioProcessor.presetManager.getPresetNames();
    // Keep the menu theme alive until the asynchronous popup has closed.
    auto menuLook = std::make_shared<CustomLookAndFeel>();
    for (const auto colourID : { juce::PopupMenu::backgroundColourId,
                                 juce::PopupMenu::textColourId,
                                 juce::PopupMenu::highlightedBackgroundColourId,
                                 juce::PopupMenu::highlightedTextColourId })
        menuLook->setColour(colourID, customLook.findColour(colourID));

    juce::PopupMenu menu;
    menu.setLookAndFeel(menuLook.get());
    menu.addItem(1, "Save preset as...");
    menu.addSeparator();
    menu.addSectionHeader("USER PRESETS");

    if (names.isEmpty())
        menu.addItem(2, "No presets saved", false);
    else
        for (int i = 0; i < names.size(); ++i)
            menu.addItem(i + 2, names[i], true,
                         names[i] == audioProcessor.presetManager.getCurrentPresetName());

    juce::Component::SafePointer<StellaverbAudioProcessorEditor> safeThis(this);
    menu.showMenuAsync(
        juce::PopupMenu::Options().withTargetComponent(&presetButton)
                                 .withStandardItemHeight(30)
                                 .withDeletionCheck(*this),
        [safeThis, names, menuLook](int result)
        {
            if (safeThis == nullptr || result == 0)
                return;

            if (result == 1)
            {
                safeThis->showSavePresetDialog();
                return;
            }

            const int index = result - 2;
            if (!juce::isPositiveAndBelow(index, names.size()))
                return;

            const auto loaded =
                safeThis->audioProcessor.presetManager.loadPreset(names[index]);

            safeThis->updatePresetButton();

            if (loaded.failed())
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Could not load preset", loaded.getErrorMessage());
        });
}

void StellaverbAudioProcessorEditor::showSavePresetDialog()
{
    if (!presetButton.isEnabled())
        return;

    auto& manager = audioProcessor.presetManager;
    const auto folder = manager.getPresetFolder();
    const auto created = folder.createDirectory();
    if (created.failed())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon, "Could not open preset folder",
            created.getErrorMessage());
        return;
    }

    const auto filename = juce::File::createLegalFileName(
        manager.getCurrentPresetName()) + ".preset";
    presetSaveChooser = std::make_unique<juce::FileChooser>(
        "Save preset as...", folder.getChildFile(filename), "*.preset", true);
    presetButton.setEnabled(false);

    juce::Component::SafePointer<StellaverbAudioProcessorEditor> safeThis(this);
    presetSaveChooser->launchAsync(
        juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::warnAboutOverwriting,
        [safeThis](const juce::FileChooser& chooser)
        {
            if (safeThis == nullptr)
                return;

            safeThis->presetButton.setEnabled(true);
            const auto destination = chooser.getResult();
            if (destination == juce::File())
                return;

            // Save to the exact path whose overwrite the native dialog confirmed.
            const auto saved = safeThis->audioProcessor.presetManager
                .savePresetToFile(destination, true);
            safeThis->updatePresetButton();

            if (saved.failed())
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon, "Could not save preset",
                    saved.getErrorMessage());
        });
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

void CustomLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                              juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto cornerSize = 4.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    auto baseColour = backgroundColour.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, cornerSize);

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour (juce::Colours::orange);
        g.drawRoundedRectangle (bounds, cornerSize, 2.0f);
    }
    else
    {
        g.setColour (juce::Colours::white.withAlpha(0.5f));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }
}

juce::Font CustomLookAndFeel::getLabelFont (juce::Label& label)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
}

juce::Font CustomLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), juce::jmin(13.0f, (float)buttonHeight * 0.6f), juce::Font::plain));
}

juce::Font CustomLookAndFeel::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
}

juce::Font CustomLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
}

juce::Path CustomLookAndFeel::getTickShape (float height)
{
    juce::Path p;
    float diameter = height * 0.01f; // make it smaller
    float x = (height * 2.0f - diameter) * 0.5f;
    float y = (height - diameter) * 0.5f;
    p.addEllipse (x, y, 1.0f, 1.0f);
    return p;
}

void CustomLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                           const bool isSeparator, const bool isActive,
                                           const bool isHighlighted, const bool isTicked,
                                           const bool hasSubMenu, const juce::String& text,
                                           const juce::String& shortcutKeyText,
                                           const juce::Drawable* icon, const juce::Colour* const textColourToUse)
{
    if (isSeparator)
    {
        auto r  = area.reduced (5, 0);
        r.removeFromTop (juce::roundToInt (((float) r.getHeight() * 0.5f) - 0.5f));

        g.setColour (findColour (juce::PopupMenu::textColourId).withAlpha (0.3f));
        g.fillRect (r.removeFromTop (1));
    }
    else
    {
        auto textColour = (textColourToUse == nullptr ? findColour (juce::PopupMenu::textColourId)
                                                      : *textColourToUse);

        auto r  = area.reduced (1);

        if (isHighlighted && isActive)
        {
            g.setColour (findColour (juce::PopupMenu::highlightedBackgroundColourId));
            g.fillRect (r);

            g.setColour (findColour (juce::PopupMenu::highlightedTextColourId));
        }
        else
        {
            g.setColour (textColour.withMultipliedAlpha (isActive ? 1.0f : 0.5f));
        }

        r.reduce (juce::jmin (5, area.getWidth() / 20), 0);

        auto font = getPopupMenuFont();

        auto maxFontHeight = (float) r.getHeight() / 1.3f;

        if (font.getHeight() > maxFontHeight)
            font.setHeight (maxFontHeight);

        g.setFont (font);

        auto iconArea = r.removeFromLeft (juce::roundToInt (maxFontHeight)).toFloat();

        if (icon != nullptr)
        {
            icon->drawWithin (g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
            r.removeFromLeft (juce::roundToInt (maxFontHeight * 0.5f));
        }
        else if (isTicked)
        {
            g.setColour(juce::Colours::orange);
            g.drawText ("*", iconArea.toNearestInt(), juce::Justification::centred, true);
            if (isHighlighted && isActive) g.setColour (findColour (juce::PopupMenu::highlightedTextColourId));
            else g.setColour (textColour.withMultipliedAlpha (isActive ? 1.0f : 0.5f));
        }

        if (hasSubMenu)
        {
            auto arrowH = 0.6f * getPopupMenuFont().getAscent();

            auto x = static_cast<float> (r.removeFromRight ((int) arrowH).getX());
            auto halfH = static_cast<float> (r.getCentreY());

            juce::Path path;
            path.startNewSubPath (x, halfH - arrowH * 0.5f);
            path.lineTo (x + arrowH * 0.6f, halfH);
            path.lineTo (x, halfH + arrowH * 0.5f);

            g.strokePath (path, juce::PathStrokeType (2.0f));
        }

        r.removeFromRight (3);
        g.drawFittedText (text, r, juce::Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            auto f2 = font;
            f2.setHeight (f2.getHeight() * 0.75f);
            f2.setHorizontalScale (0.95f);
            g.setFont (f2);

            g.drawText (shortcutKeyText, r, juce::Justification::centredRight, true);
        }
    }
}
