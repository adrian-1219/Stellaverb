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
    
    struct StarGUI
    {
        float x;
        float y;
        float z;
        float amplitude;
        float distance;
        float azimuth;
    };
    
    std::vector<StarGUI> stars;
    
    void loadStarData()
    {
        const void* rawData = BinaryData::stars_sorted_GUI_bin;
        size_t dataSize = BinaryData::stars_sorted_GUI_binSize;
        size_t count = dataSize / sizeof(StarGUI);
        stars.resize(count);
        std::memcpy(stars.data(), rawData, dataSize);
    }

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

    class StarDisplay : public juce::Component, public juce::Timer
    {
    public:
        StarDisplay(const std::vector<StarGUI>& s) : starsRef(s)
        {
            startTimerHz(30);
        }
        
        ~StarDisplay() override
        {
            stopTimer();
        }
        
        void timerCallback() override
        {
            rotation += 0.001f;
            repaint();
        }
        
        void setCurrentLength(float newLength)
        {
            currentLength = newLength;
            repaint();
        }
        
        void setMaxDistance(float distance)
        {
            maxDistance = distance;
            repaint();
        }
        
        void setMinDistance(float distance)
        {
            minDistance = distance;
            repaint();
        }
        void setMaxBrightness(float brightness)
        {
            maxBrightness = brightness;
            repaint();
        }
        
        void setMinBrightness(float brightness)
        {
            minBrightness = brightness;
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            g.setColour(juce::Colours::black);
            g.fillEllipse(getLocalBounds().toFloat());
            
            float halfW = getWidth() * 0.5f;
            float halfH = getHeight() * 0.5f;
            float cosRot = std::cos(rotation);
            float sinRot = std::sin(rotation);
            
            float scale = 1.0f; // Constant scale so it doesn't zoom in/out
            
            for (const auto& star : starsRef)
            {
                if (star.distance > maxDistance or star.distance < minDistance or star.amplitude < minBrightness or star.amplitude > maxBrightness) {
                    continue;
                }
                
                float r = star.distance;

                float cx = (star.x - 0.5f) * r * scale;
                float cy = (star.y - 0.5f) * r * scale;
                float cz = (star.z - 0.5f) * r * scale;

                // Rotate around Y axis
                float rx = cx * cosRot - cz * sinRot;
                float rz = cx * sinRot + cz * cosRot;
                
                // Tilt forward slightly (rotate around X axis)
                float tiltCos = 0.866f; // cos(30 deg)
                float tiltSin = 0.5f;   // sin(30 deg)
                
                float ry = cy * tiltCos - rz * tiltSin;
                float finalZ = cy * tiltSin + rz * tiltCos;

                // Perspective projection
                float zDistance = 1.5f;
                float zFactor = zDistance / (zDistance + finalZ);
                
                float px = halfW + (rx * halfW * zFactor * 1.5f);
                float py = halfH + (ry * halfH * zFactor * 1.5f);
                
                float alpha = juce::jlimit(0.1f, 1.0f, zFactor * 0.8f);
                g.setColour(juce::Colours::white.withAlpha(alpha));
                
                float size = 2.0f * zFactor;
                g.fillEllipse(px - size * 0.5f, py - size * 0.5f, size, size);
            }
            
            // Draw Ruler Axes
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.setFont(10.0f);
            
            float axisPadx = 20.0f;
            float axisPady = getHeight() - 20.0f;
            
            // X-axis (Bottom)
            g.drawLine(axisPadx, axisPady, getWidth() - 20.0f, axisPady, 1.0f);
            for (int i = 0; i <= 4; ++i)
            {
                float px = axisPadx + (getWidth() - axisPadx - 20.0f) * (i / 4.0f);
                g.drawLine(px, axisPady - 4.0f, px, axisPady + 4.0f, 1.0f);
                float val = currentLength * (i / 4.0f) * 5.0f;
                if (i == 0 or i == 4)
                    g.drawText(juce::String(val, 1) + " s", px - 15.0f, axisPady + 5.0f, 30.0f, 15.0f, juce::Justification::centred);
            }
            
            // Y-axis (Left)
            g.drawLine(axisPadx, 20.0f, axisPadx, axisPady, 1.0f);
            for (int i = 0; i <= 4; ++i)
            {
                float py = 20.0f + (axisPady - 20.0f) * (i / 4.0f);
                g.drawLine(axisPadx - 4.0f, py, axisPadx + 4.0f, py, 1.0f);
                float val = currentLength * ((4 - i) / 4.0f) * 5.0f;
                if (i == 0) // avoid overlapping the corner 0 with x-axis
                    g.drawText(juce::String(val, 1) + " s", axisPadx - 5.0f, py - 7.0f, 35.0f, 15.0f, juce::Justification::centredRight);
            }
        }
    private:
        const std::vector<StarGUI>& starsRef;
        float currentLength = 1.0f;
        float rotation = 0.0f;
        float maxDistance = 1.0f;
        float minDistance = 0.0f;
        float maxBrightness = 1.0f;
        float minBrightness = 0.0f;
    };

    juce::TextButton generateButton { "Generate IR" };
    IRGenerator irGenerator = IRGenerator(44100.0f, 2);
    
    StellaverbAudioProcessor& audioProcessor;
    juce::Slider length, minDistance, maxDistance, minBrightness, maxBrightness;
    juce::Slider predelay, width, dryWet, gain, lowFreq, highFreq, drive;
    juce::ComboBox irGenerationMode;
    juce::Label lengthLabel, minDistanceLabel, maxDistanceLabel, minBrightnessLabel, maxBrightnessLabel;
    juce::Label predelayLabel, widthLabel, dryWetLabel, gainLabel, lowFreqLabel, highFreqLabel, driveLabel;
    juce::Label irGenerationModeLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lengthAttachment, minDistanceAttachment, maxDistanceAttachment, minBrightnessAttachment, maxBrightnessAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> predelayAttachment, widthAttachment, dryWetAttachment, gainAttachment, lowFreqAttachment, highFreqAttachment, driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> irGenerationModeAttachment;
    
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache { 10 };
    juce::AudioThumbnail thumbnail { 512, formatManager, thumbnailCache };
    WaveformDisplay waveformDisplay { thumbnail };
    StarDisplay starDisplay { stars };
    
    CustomLookAndFeel customLook;
    
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StellaverbAudioProcessorEditor)
};
