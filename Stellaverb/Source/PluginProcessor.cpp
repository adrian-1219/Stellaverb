/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "IRGenerator.h"

//==============================================================================
StellaverbAudioProcessor::StellaverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, "parameters", createParameterLayout())
#endif
{
    formatManager.registerBasicFormats(); // WAV, AIFF, MP3, etc.
}

StellaverbAudioProcessor::~StellaverbAudioProcessor()
{
}

//==============================================================================
const juce::String StellaverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StellaverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StellaverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StellaverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StellaverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StellaverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StellaverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StellaverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StellaverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void StellaverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StellaverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dryBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    
    // Set up dsp module
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    // Convolution
    convolution.prepare(spec);
        
    // Pre delay
    preDelayL.reset();
    preDelayR.reset();
    preDelayL.prepare(spec);
    preDelayR.prepare(spec);
    preDelaySmoothed.reset(sampleRate, 0.05);
    
    // Filters
    lowPassFilter.prepare(spec);
    highPassFilter.prepare(spec);

    // Output Gain
    outputGain.prepare(spec);
    outputGain.setGainLinear(0.0f);
    
    // Load an IR file (WAV)
    juce::File irFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Stellaverb")
        .getChildFile("generated_ir.wav");
    
    DBG("IR path: " << irFile.getFullPathName());
    DBG("Exists before load: " << int(irFile.existsAsFile()));
    
    irFile.getParentDirectory().createDirectory();
    
    // If it doesn't exist, generate the default IR
    if (!irFile.existsAsFile())
    {
        DBG("IR file not found, generating new one...");
        IRGenerator irGenerator = IRGenerator(44100.0f, 2);
        irGenerator.generate();

        irGenerator.saveToWav(irFile);
    }
    
    convolution.loadImpulseResponse(irFile, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, irFile.getSize());
    irReady = true;
}

void StellaverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StellaverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void StellaverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    dryBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    dryBuffer.copyFrom(1, 0, buffer, 1, 0, buffer.getNumSamples());
    
    auto sampleRate = getSampleRate();
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    // this process block assumes stereo!!!!
    if (irReady) // only process if the IR is ready
    {
        // load parameters
        auto width = parameters.getRawParameterValue("width")->load();
        auto preDelay = parameters.getRawParameterValue("preDelay")->load();
        auto lowFrequency = parameters.getRawParameterValue("lowFreq")->load();
        auto highFrequency = parameters.getRawParameterValue("highFreq")->load();
        auto drive = parameters.getRawParameterValue("drive")->load();
        auto dryWet = parameters.getRawParameterValue("dryWet")->load();
        
        auto numSamples = block.getNumSamples();
        auto* left  = block.getChannelPointer(0);
        auto* right = block.getChannelPointer(1);

        preDelaySmoothed.setTargetValue(preDelay);
        
        for (size_t i = 0; i < numSamples; ++i)
        {
            // pre delay (with smoothing to prevent clicks when changing parameter)
            float smoothedPreDelay = preDelaySmoothed.getNextValue();
            float preDelaySamples = smoothedPreDelay * 0.001f * (float) sampleRate;
            preDelayL.setDelay(preDelaySamples);
            preDelayR.setDelay(preDelaySamples);
            
            float inL = left[i];
            float inR = right[i];
            
            preDelayL.pushSample(0, inL);
            preDelayR.pushSample(0, inR);
            
            left[i]  = preDelayL.popSample(0);
            right[i] = preDelayR.popSample(0);
        }
        
        // convolution
        convolution.process(context);
        
        // eq
        *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, lowFrequency, 0.707f);
        *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, highFrequency, 0.707f);
        highPassFilter.process(context);
        lowPassFilter.process(context);
        
        auto* dryL = dryBuffer.getReadPointer(0);
        auto* dryR = dryBuffer.getReadPointer(1);
        
        for (size_t i = 0; i < numSamples; ++i)
        {
            // saturation
            float saturationGainComp = 1.0f / std::sqrt(drive);
            left[i]  = juce::dsp::FastMathApproximations::tanh(left[i] * drive) * saturationGainComp;
            right[i] = juce::dsp::FastMathApproximations::tanh(right[i] * drive) * saturationGainComp;
            
            // stereo widening using mid/side widening
            float L = left[i];
            float R = right[i];
            
            float mid  = 0.5f * (L + R);
            float side = 0.5f * (L - R);
            
            side *= width;
            
            float wideningGainComp = 1.0f / juce::jmax(1.0f, width);
            
            left[i]  = (mid + side) * wideningGainComp;
            right[i] = (mid - side) * wideningGainComp;
            
            // dry / wet
            const float wetComp = 4.0f; // wet gain compensation since convolution is not level neutral
            left[i] = dryL[i] * (1 - dryWet) + left[i] * dryWet * wetComp;
            right[i] = dryR[i] * (1 - dryWet) + right[i] * dryWet * wetComp;
        }
        
        // output gain
        auto gainDb = parameters.getRawParameterValue("gain")->load();
        float gainLinear = juce::Decibels::decibelsToGain(gainDb);
        outputGain.setGainLinear(gainLinear);
        
        
    }
    else // mute otherwise
    {
        outputGain.setGainLinear(0.0f);
    }
    outputGain.process(context);
}

//==============================================================================
bool StellaverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StellaverbAudioProcessor::createEditor()
{
    return new StellaverbAudioProcessorEditor (*this);
}

//==============================================================================
void StellaverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void StellaverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StellaverbAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout StellaverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"length", 1}, "Length", 0.05f, 5.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"decayTime", 1}, "DecayTime", 0.05f, 10.0f, 5.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"minDistance", 1}, "MinDistance", 0.0f, 1.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"maxDistance", 1}, "MaxDistance", 0.0f, 1.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"minBrightness", 1}, "MinBrightness", 0.0f, 1.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"maxBrightness", 1}, "MaxBrightness", 0.0f, 1.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"preDelay", 1}, "Predelay", 0.0f, 200.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"width", 1}, "Width", 0.01f, 2.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"dryWet", 1}, "Dry/Wet", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"gain", 1}, "Gain", -60.0f, 12.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowFreq", 1}, "LowFreq", 20.0f, 500.0f, 20.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highFreq", 1}, "HighFreq", 2000.0f, 20000.0f, 20000.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"drive", 1}, "Drive", 1.0f, 15.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"irGenerationMode", 1}, "IRGenerationMode", juce::StringArray{"Acoustic", "Reverse", "Spectral"}, 0));
    
    return {parameters.begin(), parameters.end()};
}
