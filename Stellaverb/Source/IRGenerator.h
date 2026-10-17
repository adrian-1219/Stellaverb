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
    void setMaxBrightness(float brightness)
    {
        maxBrightness = brightness;
    }
    
    void setMinBrightness(float brightness)
    {
        minBrightness = brightness;
    }
    
    void setMode(int mode)
    {
        generationMode = mode;
    }

    void generate()
    {
        
        impulseResponse.setSize(numChannels, irLengthSamples);
        impulseResponse.clear();

        juce::Random rng;

        auto* left = impulseResponse.getWritePointer(0);
        auto* right = impulseResponse.getWritePointer(1);
        
        std::fill(left, left + irLengthSamples, 0.0f);
        std::fill(right, right + irLengthSamples, 0.0f);
        
//        DBG("Stars: " << stars.size());
        
        for (const auto& s : stars)
        {
            if (s.distance < minDistance or s.amplitude < minBrightness or s.amplitude > maxBrightness) {
                continue;
            }
            if (s.distance > maxDistance)
            {
                break; // Since the dataset is sorted by distance, we don't have to check any of the stars after
            }
        
            const float norm = (s.distance - minDistance) / (maxDistance - minDistance);
            int idx = (int)(norm * (irLengthSamples - 1));
            
            if (idx >= 0 && idx < irLengthSamples)
            {
                float sample;
                if (generationMode == 1)
                {
                    sample = s.distance;
                }
                else if (generationMode == 0)
                {
                    sample = 1.0f - s.distance;
                }
                else
                {
                    sample = s.amplitude;
                }
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
                float env = 1;
                if (generationMode == 0)
                {
                    env = std::exp(-3.0f * t);
                }
                else if (generationMode == 1)
                {
                    env = std::exp(-3.0f * (1-t));
                }
                
                data[i] = data[i] * env;
            }
        }
        normalize();
    
    }

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
        float x;
        float y;
        float z;
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
    int irLengthSamples = 44100 * 1.0f;

    
    // default parameters
    // use all stars as default
    float decay = 1.0f;
    float maxDistance = 1.0f;
    float minDistance = 0.0f;
    float maxBrightness = 1.0f;
    float minBrightness = 0.0f;
    int generationMode = 0;

    juce::AudioBuffer<float> impulseResponse;


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
