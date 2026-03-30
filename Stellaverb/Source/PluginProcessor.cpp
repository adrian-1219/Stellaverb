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
    // Load audio file for testing purposes

    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();
    
    juce::File file = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                        .getChildFile("test.wav");

    auto *reader = formatManager.createReaderFor(file); // Read the file...

    if (reader != nullptr) // if the reader is not empty
    {
        DBG("File Loaded Successfully: " << file.getFileName());
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true); // starting te variable who is gonna store the content.
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.swap(newSource);
        transportSource.start();
    }
    else
    {
        DBG("Failed to create reader for: " << file.getFileName());
    }
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
        
    
    // Convolution
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    convolution.prepare(spec);
    
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
        irGenerator.setLengthSeconds(2.5f);
        irGenerator.setDecay(4.0f);
        irGenerator.setNumReflections(20);
        irGenerator.setLowpassFrequency(5000.0f);

        irGenerator.generate();

        irGenerator.saveToWav(irFile);
    }
    
    convolution.loadImpulseResponse(irFile, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, irFile.getSize());
}

void StellaverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();
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
    if (readerSource != nullptr)
    {
        if (!transportSource.isPlaying())
            transportSource.start();
        
        juce::AudioSourceChannelInfo loadedFile(buffer);
        transportSource.getNextAudioBlock(loadedFile);
    }
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    convolution.process(context);
    
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

//    // Buttons
//    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"file", 1}, "File", 0));
//    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"play", 1}, "Play", 0));
//    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"stop", 1}, "Stop", 0));
//    // Sliders
//    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highGain", 1}, "HighGain", -40.0f, 20.0f, 0.5f));
//    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highQ", 1}, "HighQ", 0.1f, 10.0f, 0.5f));
//    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highFrequency", 1}, "highFrequency", 450.0f, 18000.0f, 3000.0f));
//    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowGain", 1}, "lowGain", -40.0f, 20.0f, 0.5f));
//    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowQ", 1}, "LowQ", 0.1f, 10.0f, 0.5f));
//    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowFrequency", 1}, "LowFrequency", 10.0f, 600.0f, 150.0f));
    
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"length", 1}, "Length", 0.05f, 5.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"decayTime", 1}, "DecayTime", 0.05f, 5.0, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"numReflections", 1}, "NumReflections", 1, 100, 10));

    return {parameters.begin(), parameters.end()};
}
