/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "IRGenerator.h"
#include <array>
#include "PresetManager.h"

//==============================================================================
/**
*/
class StellaverbAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    StellaverbAudioProcessor();
    ~StellaverbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;
    
    PresetManager presetManager {
        *this,
        "adrianlam.stellaverb",
        juce::File::getSpecialLocation(
            juce::File::userApplicationDataDirectory)
            .getChildFile("Stellaverb")
            .getChildFile("Presets")
    };
    
    // IR params
    std::atomic<float>* lengthParam;
    std::atomic<float>* minDistanceParam;
    std::atomic<float>* maxDistanceParam;
    std::atomic<float>* minBrightnessParam;
    std::atomic<float>* maxBrightnessParam;
    std::atomic<float>* irGenerationModeParam;
    
    // Effects params
    std::atomic<float>* widthParam;
    std::atomic<float>* preDelayParam;
    std::atomic<float>* lowFreqParam;
    std::atomic<float>* highFreqParam;
    std::atomic<float>* driveParam;
    std::atomic<float>* dryWetParam;

    // JUCE dsp modules
    juce::dsp::Convolution convolution {juce::dsp::Convolution::NonUniform { 1024 }};
    juce::dsp::Gain<float> outputGain;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelayL { 192000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelayR { 192000 };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> preDelaySmoothed;
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    juce::dsp::ProcessorDuplicator<Filter, Coefficients> lowPassFilter;
    juce::dsp::ProcessorDuplicator<Filter, Coefficients> highPassFilter;
    
    juce::AudioBuffer<float> dryBuffer;
    juce::AudioFormatManager formatManager;
    
    std::atomic<bool> irReady { false };
    
    // UI-thread access only while generation remains on the UI thread.
    const juce::AudioBuffer<float>& getGeneratedIRBuffer() const
    {
        return generatedIRBuffer;
    }
    
    bool copyIRForDisplay(
        juce::AudioBuffer<float>& destination,
        juce::uint64& lastSeenVersion);
    

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StellaverbAudioProcessor)
    
    static constexpr double irSampleRate = 44100.0;
    
    IRGenerator generator { irSampleRate, 2 };

    using IRSettings = std::array<float, 6>;

    IRSettings readIRSettings() const;
    juce::AudioBuffer<float> buildIR(const IRSettings& settings);
    bool generateIR();
    
    float lastLowFrequency = -1.0f;
    float lastHighFrequency = -1.0f;

    // Serialises startup generation and worker generation.
    juce::CriticalSection generationLock;
    IRSettings lastGeneratedSettings {};
    bool hasGeneratedIR = false;

    // Waveform cache shared with the editor.
    juce::CriticalSection displayLock;
    juce::AudioBuffer<float> generatedIRBuffer;
    juce::uint64 displayVersion = 0;

    juce::SpinLock pendingIRLock;
    juce::AudioBuffer<float> pendingIR;

    class IRWorker final : public juce::Thread
    {
    public:
        explicit IRWorker(StellaverbAudioProcessor& processor)
            : juce::Thread("Stellaverb IR worker"),
              owner(processor)
        {
        }

        void run() override
        {
            while (!threadShouldExit())
            {
                owner.generateIR();
                wait(200);
            }
        }

    private:
        StellaverbAudioProcessor& owner;
    };

    // Keep this after the members the worker accesses.
    IRWorker irWorker { *this };
};
