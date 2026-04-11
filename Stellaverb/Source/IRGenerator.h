/*
  ==============================================================================

    StarsToIR.h
    Created: 30 Mar 2026 10:01:53am
    Author:  Adrian Lam

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <BinaryData.h>

class IRGenerator
{
public:
    IRGenerator(double sampleRate, int numChannels = 2)
        : sampleRate(sampleRate), numChannels(numChannels)
    {
        loadStarData();
    }

    void setLengthSeconds(float seconds)
    {
        irLengthSamples = static_cast<int>(sampleRate * seconds);
    }

    void setDecay(float decayTime)
    {
        decay = decayTime;
    }

    void setMaxDistance(float distance)
    {
        maxDistance = distance;
    }
    
    void setMinDistance(float distance)
    {
        minDistance = distance;
    }

    // ==============================
    // Generate IR
    // ==============================
    void generate()
    {
        
        impulseResponse.setSize(numChannels, irLengthSamples);
        impulseResponse.clear();

        juce::Random rng;

        auto* left = impulseResponse.getWritePointer(0);
        auto* right = impulseResponse.getWritePointer(1);
        
        std::fill(left, left + irLengthSamples, 0.0f);
        std::fill(right, right + irLengthSamples, 0.0f);
        
        DBG("Stars: " << stars.size());
        
        for (const auto& s : stars)
        {
            if (s.distance < minDistance) {
                continue;
            }
            if (s.distance > maxDistance)
            {
                break; // Since the dataset is sorted by distance, we don't have to check any of the stars after
            }
            const int idx = (int)((s.distance - minDistance) / (maxDistance - minDistance) * (irLengthSamples - 1));
            if (idx >= 0 && idx < irLengthSamples)
            {
                float sample = s.amplitude;

                left[idx]  += sample * s.left;
                right[idx] += sample * s.right;
            }
           
        }
        
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = impulseResponse.getWritePointer(ch);

            for (int i = 0; i < irLengthSamples; ++i)
            {

                float t = (float)i / (float)irLengthSamples;

                // Exponential decay
                float env = std::exp(-decay * t);

                data[i] = data[i] * env;
            }
        }

//        addEarlyReflections();
//        applyLowpassFilter();
        normalize();
        
        DBG("first 100 samples L/R diff:");

        for (int i = 0; i < 100; ++i)
        {
            DBG(juce::String(i) + ": "
                + juce::String(left[i]) + " / "
                + juce::String(right[i]));
        }
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
    
    struct Star
    {
        float left;
        float right;
        float amplitude;
        float distance;
        float azimuth;
    };
    
    std::vector<Star> stars;
    
    void loadStarData()
    {
        const void* rawData = BinaryData::stars_sorted_bin;
        size_t dataSize = BinaryData::stars_sorted_binSize;

        size_t count = dataSize / sizeof(Star);

        stars.resize(count);

        std::memcpy(stars.data(), rawData, dataSize);
    }


private:
    double sampleRate = 44100.0;
    int numChannels = 2;
    int irLengthSamples = 44100 * 2;

    
    // default parameters
    float decay = 3.0f;
    float maxDistance = 1.0f; // use all stars
    float minDistance = 0.0f; // use all stars
    float lowpassFreq = 6000.0f;
    float speed = 200.0f;

    juce::AudioBuffer<float> impulseResponse;

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
