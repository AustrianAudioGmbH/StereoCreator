/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StereoCreatorAudioProcessor::StereoCreatorAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
params(*this, nullptr, "StereoCreator", {
    std::make_unique<AudioParameterInt> ("stereoMode", "Stereo Mode", 1, 2, 1, "",
                                           [](int value, int maximumStringLength) {return String(value + 1);}, nullptr),
    std::make_unique<AudioParameterFloat> ("msMidGain", "MS Mid Gain", NormalisableRange<float>( - 32.0f, 3.0f, 0.1f), 0.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("msSideGain", "MS Side Gain", NormalisableRange<float>( - 32.0f, 3.0f, 0.1f), 0.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("lrWidth", "LR Width", NormalisableRange<float>( 0.0f, 100.0f, 1.0f), 100.0f,  "%", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
    std::make_unique<AudioParameterBool>("channelSwitch", "Channel Switch", false, "", [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
    std::make_unique<AudioParameterBool>("autoLevelMode", "Auto Level Mode", false, "", [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
    std::make_unique<AudioParameterFloat> ("msMidPattern", "MS Mid Pattern", NormalisableRange<float> (- 1.0f, 1.0f, 0.01f), 0.0f, "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 2); }, nullptr)
})
{
    params.addParameterListener("stereoMode", this);
    params.addParameterListener("msMidGain", this);
    params.addParameterListener("msSideGain", this);
    params.addParameterListener("channelSwitch", this);
    params.addParameterListener("autoLevelMode", this);
    params.addParameterListener("msMidPattern", this);
    
    stereoModeIdx = params.getRawParameterValue("stereoMode");
    channelSwitchOn = params.getRawParameterValue("channelSwitch");
    autoLevelsOn = params.getRawParameterValue("autoLevelMode");
}

StereoCreatorAudioProcessor::~StereoCreatorAudioProcessor()
{
}

//==============================================================================
const juce::String StereoCreatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StereoCreatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StereoCreatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StereoCreatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StereoCreatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StereoCreatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StereoCreatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StereoCreatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StereoCreatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void StereoCreatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StereoCreatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    numInputs = getTotalNumInputChannels();
    
//    if (getTotalNumInputChannels() != 2 && getTotalNumInputChannels() != 4)
//        wrongBusConfiguration = true;
//    else
//        wrongBusConfiguration = false;
    
    omniEightLrBuffer.setSize(2, currentBlockSize);
    omniEightLrBuffer.clear();
    omniEightFbBuffer.setSize(2, currentBlockSize);
    omniEightFbBuffer.clear();
    msLeftRightBuffer.setSize(2, currentBlockSize);
    msLeftRightBuffer.clear();
    msMidBuffer.setSize(1, currentBlockSize);
    msMidBuffer.clear();
    chSwitchBuffer.setSize(2, currentBlockSize);
    chSwitchBuffer.clear();
    passThroughLeftRightBuffer.setSize(2, currentBlockSize);
    passThroughLeftRightBuffer.clear();
    
    for (int i = 0; i < panTableSize; i++)
    {
        panTableLeft[i] = std::powf(cos(MathConstants<float>::pi / 2.0f * ((float) i / ((float) panTableSize))), panLawExp);
        panTableRight[i] = std::powf(sin(MathConstants<float>::pi / 2.0f * ((float) i / ((float) panTableSize - 1.0f))), panLawExp);
//        panTableLeft[i] = cos((float) i / ((float) panTableSize));
//        panTableRight[i] = sin((float) i / ((float) panTableSize - 1.0f));
    }

}

void StereoCreatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool StereoCreatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if ((layouts.getMainOutputChannelSet() != AudioChannelSet::stereo()
        || layouts.getMainInputChannelSet() != AudioChannelSet::stereo())
        && layouts.getMainInputChannelSet() != AudioChannelSet::quadraphonic())
        return false;
    
    if (layouts.getMainInputChannelSet().isDisabled())
        return false;
    
    if (layouts.getMainOutputChannelSet().isDisabled())
        return false;
    
    return true;
}

void StereoCreatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
//    auto totalNumOutputChannels = getTotalNumOutputChannels();

    int numSamples = buffer.getNumSamples();
    
    const float* readPointerLeft = buffer.getReadPointer(0);
    const float* readPointerRight = buffer.getReadPointer(1);
    
    float* writePointerPassThroughLeft = passThroughLeftRightBuffer.getWritePointer(0);
    float* writePointerPassThroughRight = passThroughLeftRightBuffer.getWritePointer(1);
    
    float* writePointerOmniLR = omniEightLrBuffer.getWritePointer(0);
    float* writePointerEightLR = omniEightLrBuffer.getWritePointer(1);
    
    float* writePointerMsLeft = msLeftRightBuffer.getWritePointer(0);
    float* writePointerMsRight = msLeftRightBuffer.getWritePointer(1);
    
    FloatVectorOperations::copy (writePointerOmniLR, readPointerLeft, numSamples);
    FloatVectorOperations::add (writePointerOmniLR, readPointerRight, numSamples);
    
    FloatVectorOperations::copy (writePointerEightLR, readPointerLeft, numSamples);
    FloatVectorOperations::subtract (writePointerEightLR, readPointerRight, numSamples);
    
    // one OC-818 was used, ms is calculated with left/right signals
    if (totalNumInputChannels == 2)
    {
        switch ((int) stereoModeIdx->load())
        {
            case eStereoMode::msIdx: // when ms is chosen
                // creating omni and eight patterns
                FloatVectorOperations::multiply(writePointerOmniLR, Decibels::decibelsToGain(params.getRawParameterValue("msMidGain")->load()), numSamples);
                FloatVectorOperations::multiply(writePointerEightLR, Decibels::decibelsToGain(params.getRawParameterValue("msSideGain")->load()), numSamples);
                FloatVectorOperations::copy(writePointerMsLeft, writePointerOmniLR, numSamples);
                FloatVectorOperations::add(writePointerMsLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerMsRight, writePointerOmniLR, numSamples);
                FloatVectorOperations::subtract(writePointerMsRight, writePointerEightLR, numSamples);
                
                buffer.copyFrom(0, 0, msLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, msLeftRightBuffer, 1, 0, numSamples);
                break;
            case eStereoMode::passThroughIdx: // when pass through is chosen
                // adding up the cardioids according to the width setting
                FloatVectorOperations::copy(writePointerPassThroughLeft, readPointerLeft, numSamples);
                FloatVectorOperations::multiply(writePointerPassThroughLeft, 1.0f - params.getParameter("lrWidth")->convertTo0to1(params.getRawParameterValue("lrWidth")->load()), numSamples);
                
                FloatVectorOperations::copy(writePointerPassThroughRight, readPointerRight, numSamples);
                FloatVectorOperations::multiply(writePointerPassThroughRight, 1.0f - params.getParameter("lrWidth")->convertTo0to1(params.getRawParameterValue("lrWidth")->load()), numSamples);
                
                buffer.addFrom(0, 0, passThroughLeftRightBuffer, 1, 0, numSamples);
                buffer.addFrom(1, 0, passThroughLeftRightBuffer, 0, 0, numSamples);
                
                // compensating gain difference of cardioids vs omni
                buffer.applyGain(0, 0, numSamples, 1.0f - (1.0f - params.getParameter("lrWidth")->convertTo0to1(params.getRawParameterValue("lrWidth")->load())) / 3.0f);
                buffer.applyGain(1, 0, numSamples, 1.0f - (1.0f - params.getParameter("lrWidth")->convertTo0to1(params.getRawParameterValue("lrWidth")->load())) / 3.0f);
                break;
                
            default:
                break;
        }

    }
    // two OC-818 were used, ms is calculated from left/right and front/back signals
    else if (totalNumInputChannels == 4)
    {
        const float* readPointerFront = buffer.getReadPointer(2);
        const float* readPointerBack = buffer.getReadPointer(3);
        
        float* writePointerMsMid = msMidBuffer.getWritePointer(0);
        
        float* writePointerOmniFB = omniEightFbBuffer.getWritePointer(0);
        float* writePointerEightFB = omniEightFbBuffer.getWritePointer(1);
        
        FloatVectorOperations::copy (writePointerOmniFB, readPointerFront, numSamples);
        FloatVectorOperations::add (writePointerOmniFB, readPointerBack, numSamples);
        
        FloatVectorOperations::copy (writePointerEightFB, readPointerFront, numSamples);
        FloatVectorOperations::subtract (writePointerEightFB, readPointerBack, numSamples);
        
        switch ((int) stereoModeIdx->load())
        {
            case eStereoMode::msIdx:
                FloatVectorOperations::copy(writePointerMsMid, readPointerBack, numSamples);
                FloatVectorOperations::multiply(writePointerMsMid, params.getRawParameterValue("msMidPattern")->load(), numSamples);
                FloatVectorOperations::add(writePointerMsMid, readPointerFront, numSamples);
                
                FloatVectorOperations::multiply(writePointerMsMid, Decibels::decibelsToGain(params.getRawParameterValue("msMidGain")->load()), numSamples);
                FloatVectorOperations::multiply(writePointerEightLR, Decibels::decibelsToGain(params.getRawParameterValue("msSideGain")->load()), numSamples);
                
                FloatVectorOperations::copy(writePointerMsLeft, writePointerMsMid, numSamples);
                FloatVectorOperations::add(writePointerMsLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerMsRight, writePointerMsMid, numSamples);
                FloatVectorOperations::subtract(writePointerMsRight, writePointerEightLR, numSamples);
                


                
                buffer.copyFrom(0, 0, msLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, msLeftRightBuffer, 1, 0, numSamples);
                buffer.clear(2, 0, numSamples);
                buffer.clear(3, 0, numSamples);
                break;
                
            default:
                break;
        }
    }
    else
    {
        return;
    }
    
    if (channelSwitchOn->load() >= 0.5f)
    {
        chSwitchBuffer.copyFrom(0, 0, buffer, 1, 0, numSamples);
        chSwitchBuffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
        buffer.copyFrom(0, 0, chSwitchBuffer, 0, 0, numSamples);
        buffer.copyFrom(1, 0, chSwitchBuffer, 1, 0, numSamples);
    }
    
}

//==============================================================================
bool StereoCreatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StereoCreatorAudioProcessor::createEditor()
{
    return new StereoCreatorAudioProcessorEditor (*this, params);
}

//==============================================================================
void StereoCreatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void StereoCreatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void StereoCreatorAudioProcessor::parameterChanged(const String &parameterID, float newValue)
{
    if (parameterID == "msSideGain" && autoLevelsOn->load() >= 0.5f)
    {
        getMidGain(Decibels::decibelsToGain(params.getRawParameterValue("msSideGain")->load()));
        sideGainChanged = false;
    }
    else if (parameterID == "msMidGain" && autoLevelsOn->load() >= 0.5f)
    {
        getSideGain(Decibels::decibelsToGain(params.getRawParameterValue("msMidGain")->load()));
        midGainChanged = false;
        
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoCreatorAudioProcessor();
}


void StereoCreatorAudioProcessor::getMidGain(float sideGain)
{
    sideGainChanged = true;
    if (midGainChanged) { return; }
    
    int panIdx = 0;
    
    for (int i = 0; i < panTableSize; i++)
    {
        if (sideGain <= panTableLeft[i] + 0.0001f && sideGain >= panTableLeft[i] - 0.0001f)
        {
            panIdx = i;
        }
    }
    params.getParameter("msMidGain")->setValueNotifyingHost (params.getParameter("msMidGain")->convertTo0to1(Decibels::gainToDecibels(panTableRight[panIdx])));
    
    
}

void StereoCreatorAudioProcessor::getSideGain(float midGain)
{
    midGainChanged = true;
    if (sideGainChanged) { return; }
    
    int panIdx = panTableSize - 1;
    
    for (int i = 0; i < panTableSize; i++)
    {
        if (midGain <= panTableRight[i] + 0.0001f && midGain >= panTableRight[i] - 0.0001f)
        {
            panIdx = i;
        }
    }
    params.getParameter("msSideGain")->setValueNotifyingHost (params.getParameter("msSideGain")->convertTo0to1(Decibels::gainToDecibels(panTableLeft[panIdx])));
}
