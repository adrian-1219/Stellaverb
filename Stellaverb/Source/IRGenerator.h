/*
  ==============================================================================

    StarsToIR.h
    Created: 30 Mar 2026 10:01:53am
    Author:  Adrian Lam

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class IRGenerator
{
public:
    IRGenerator(double sampleRate, int numChannels = 2)
        : sampleRate(sampleRate), numChannels(numChannels)
    {
    }

    void setLengthSeconds(float seconds)
    {
        irLengthSamples = static_cast<int>(sampleRate * seconds);
    }

    void setDecay(float decayTime)
    {
        decay = decayTime;
    }

    void setNumReflections(int reflections)
    {
        numReflections = reflections;
    }

    void setLowpassFrequency(float freq)
    {
        lowpassFreq = freq;
    }

    // ==============================
    // Generate IR
    // ==============================
    void generate()
    {
        impulseResponse.setSize(numChannels, irLengthSamples);
        impulseResponse.clear();

        juce::Random rng;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = impulseResponse.getWritePointer(ch);

            for (int i = 0; i < irLengthSamples; ++i)
            {
                float noise = rng.nextFloat() * 2.0f - 1.0f;

                float t = (float)i / (float)irLengthSamples;

                // Exponential decay
                float env = std::exp(-decay * t);

                data[i] = noise * env;
            }
        }

        addEarlyReflections();
//        applyLowpassFilter();
        normalize();
    }

    // ==============================
    // Save to WAV
    // ==============================
    bool saveToWav(const juce::File& file)
    {
        if (!file.deleteFile())
            return false;

        juce::WavAudioFormat format;

        std::unique_ptr<juce::FileOutputStream> stream(file.createOutputStream());

        if (stream == nullptr)
            return false;

        std::unique_ptr<juce::AudioFormatWriter> writer(
            format.createWriterFor(stream.get(),
                                   sampleRate,
                                   (unsigned int)numChannels,
                                   24,
                                   {},
                                   0));

        if (writer == nullptr)
            return false;

        stream.release(); // writer owns it now

        writer->writeFromAudioSampleBuffer(
            impulseResponse,
            0,
            impulseResponse.getNumSamples()
        );

        return true;
    }

    // ==============================
    // Access buffer (for convolution)
    // ==============================
    const juce::AudioBuffer<float>& getBuffer() const
    {
        return impulseResponse;
    }

private:
    double sampleRate = 44100.0;
    int numChannels = 2;
    int irLengthSamples = 44100 * 2;

    float decay = 3.0f;
    int numReflections = 12;
    float lowpassFreq = 6000.0f;

    juce::AudioBuffer<float> impulseResponse;

    // ==============================
    void addEarlyReflections()
    {
        juce::Random rng;

        for (int i = 0; i < numReflections; ++i)
        {
            int delay = rng.nextInt(irLengthSamples / 3);
            float gain = rng.nextFloat() * 0.6f;

            for (int ch = 0; ch < numChannels; ++ch)
                impulseResponse.addSample(ch, delay, gain);
        }
    }

    // ==============================
//    void applyLowpassFilter()
//    {
//        juce::dsp::IIR::Filter<float> filter;
//        *filter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lowpassFreq);
//
//        for (int ch = 0; ch < numChannels; ++ch)
//        {
//            float* data = impulseResponse.getWritePointer(ch);
//
//            for (int i = 0; i < irLengthSamples; ++i)
//                data[i] = filter.processSample(data[i]);
//        }
//    }

    // ==============================
    void normalize()
    {
        float maxVal = 0.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* data = impulseResponse.getReadPointer(ch);

            for (int i = 0; i < irLengthSamples; ++i)
                maxVal = std::max(maxVal, std::abs(data[i]));
        }

        if (maxVal > 0.0f)
            impulseResponse.applyGain(1.0f / maxVal);
    }
};
