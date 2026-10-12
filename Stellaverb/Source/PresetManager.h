/*
  ==============================================================================

    PresetManager.h
    Created: 12 Oct 2026 6:18:45pm
    Author:  Adrian Lam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <limits>

class PresetManager
{
public:
    PresetManager(juce::AudioProcessor& owner,
                  const juce::String& uniquePluginID,
                  const juce::File& presetFolder)
        : processor(owner),
          pluginID(uniquePluginID),
          folder(presetFolder)
    {
    }

    juce::Result savePreset(const juce::String& name,
                            bool overwrite = false)
    {
        const auto cleanName = name.trim();

        if (!isValidName(cleanName))
            return juce::Result::fail(
                "Use letters, numbers, spaces, hyphens or underscores.");

        const auto directoryResult = folder.createDirectory();

        if (directoryResult.failed())
            return directoryResult;

        const auto file = getPresetFile(cleanName);

        if (file.exists() && !overwrite)
            return juce::Result::fail("A preset with this name already exists.");

        juce::MemoryBlock state;
        processor.getStateInformation(state);

        if (state.getSize() == 0)
            return juce::Result::fail("The plugin returned an empty state.");

        juce::XmlElement xml("PluginPreset");
        xml.setAttribute("pluginID", pluginID);
        xml.setAttribute("formatVersion", 1);
        xml.setAttribute("state", state.toBase64Encoding());

        // Finish writing before replacing an existing preset.
        juce::TemporaryFile temporary(file);

        if (!temporary.getFile().replaceWithText(xml.toString()))
            return juce::Result::fail("Could not write the preset.");

        if (!temporary.overwriteTargetFileWithTemporary())
            return juce::Result::fail("Could not replace the preset file.");

        setCurrentPresetName(cleanName);
        return juce::Result::ok();
    }

    juce::Result loadPreset(const juce::String& name)
    {
        const auto cleanName = name.trim();

        if (!isValidName(cleanName))
            return juce::Result::fail("Invalid preset name.");

        const auto file = getPresetFile(cleanName);

        if (!file.existsAsFile())
            return juce::Result::fail("Preset not found.");

        auto xml = juce::XmlDocument::parse(file);

        if (xml == nullptr || !xml->hasTagName("PluginPreset"))
            return juce::Result::fail("Invalid preset file.");

        if (xml->getStringAttribute("pluginID") != pluginID)
            return juce::Result::fail("This preset belongs to another plugin.");

        if (xml->getIntAttribute("formatVersion") != 1)
            return juce::Result::fail("Unsupported preset format.");

        juce::MemoryBlock state;

        if (!state.fromBase64Encoding(xml->getStringAttribute("state"))
            || state.getSize() == 0
            || state.getSize() >
                static_cast<size_t>(std::numeric_limits<int>::max()))
        {
            return juce::Result::fail("Invalid preset state.");
        }

        processor.setStateInformation(
            state.getData(),
            static_cast<int>(state.getSize()));

        setCurrentPresetName(cleanName);
        return juce::Result::ok();
    }

    juce::String getCurrentPresetName() const
    {
        const juce::ScopedLock lock(nameLock);
        return currentPresetName;
    }

    void setCurrentPresetName(const juce::String& name)
    {
        const juce::ScopedLock lock(nameLock);
        currentPresetName = name.isNotEmpty() ? name : "Default";
    }

    juce::StringArray getPresetNames() const
    {
        juce::StringArray names;

        const auto files = folder.findChildFiles(
            juce::File::findFiles, false, "*.preset");

        for (const auto& file : files)
            names.add(file.getFileNameWithoutExtension());

        names.sort(true);
        return names;
    }

private:
    static bool isValidName(const juce::String& name)
    {
        return name.isNotEmpty()
            && name.length() <= 80
            && name.containsOnly(
                "abcdefghijklmnopqrstuvwxyz"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "0123456789 _-");
    }

    juce::File getPresetFile(const juce::String& name) const
    {
        return folder.getChildFile(name + ".preset");
    }

    mutable juce::CriticalSection nameLock;
    juce::String currentPresetName { "Default" };

    juce::AudioProcessor& processor;
    juce::String pluginID;
    juce::File folder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
