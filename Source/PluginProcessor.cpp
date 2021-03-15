/*
 ==============================================================================
 PluginProcessor.cpp
 Author: Simon Beck
 
 Copyright (c) 2019 - Austrian Audio GmbH
 www.austrian.audio
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    std::make_unique<AudioParameterInt> ("stereoMode", "Stereo Mode", 1, 5, 1, "",
                                           [](int value, int maximumStringLength) {return String(value + 1);}, nullptr),
    std::make_unique<AudioParameterFloat> ("msMidGain", "MS Mid Gain", NormalisableRange<float>( - 18.0f, 3.0f, 0.1f), -6.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("msSideGain", "MS Side Gain", NormalisableRange<float>( - 18.0f, 3.0f, 0.1f), -6.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("pseudoStPattern", "Pseudo-Stereo Pattern", NormalisableRange<float> (0.0f, 0.75f, 0.01f), 0.5f,  "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
    std::make_unique<AudioParameterBool>("channelSwitch", "Channel Swap", false, "", [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
    std::make_unique<AudioParameterBool>("calcCompGain", "Calculate Compensation Gain", false, "", [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
    std::make_unique<AudioParameterFloat> ("msMidPattern", "MS Mid Pattern", NormalisableRange<float> (0.0f, 0.75f, 0.01f), 0.5f, "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
    std::make_unique<AudioParameterFloat> ("trueStXyPattern", "True-Stereo Pattern", NormalisableRange<float> (0.37f, 0.75f, 0.01f), 0.5f, "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
    std::make_unique<AudioParameterFloat> ("trueStXyAngle", "True-Stereo XY Angle", NormalisableRange<float> (30.0f, 150.0f, 0.5f), 90.0f, "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("blumleinRot", "Blumlein Rotation", NormalisableRange<float> (- 30.0f, 30.0f, 0.5f), 0.0f, "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("compensationGain1", "Compensation Gain - Pseudo-MS", NormalisableRange<float>( - 9.0f, 9.0f, 0.1f), 0.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("compensationGain2", "Compensation Gain - Pseudo-Stereo", NormalisableRange<float>( - 9.0f, 9.0f, 0.1f), 0.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("compensationGain3", "Compensation Gain - True-MS", NormalisableRange<float>( - 9.0f, 9.0f, 0.1f), 0.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("compensationGain4", "Compensation Gain - True-Stereo", NormalisableRange<float>( - 9.0f, 9.0f, 0.1f), 0.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("compensationGain5", "Compensation Gain - Blumlein", NormalisableRange<float>( - 9.0f, 9.0f, 0.1f), 0.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr)

}),
layerA(nodeA), layerB(nodeB), allValueTreeStates(allStates)
{
    params.addParameterListener("stereoMode", this);
    params.addParameterListener("msMidGain", this);
    params.addParameterListener("msSideGain", this);
    params.addParameterListener("channelSwitch", this);
    params.addParameterListener("calcCompGain", this);
    params.addParameterListener("msMidPattern", this);
    params.addParameterListener("trueStXyPattern", this);
    params.addParameterListener("trueStXyAngle", this);
    params.addParameterListener("blumleinRot", this);
    
    for (int i = 0; i < 5; i++)
    {
        params.addParameterListener("compensationGain"+String(i+1), this);
    }
    
    stereoModeIdx = params.getRawParameterValue("stereoMode");
    channelSwitchOn = params.getRawParameterValue("channelSwitch");
    autoLevelsOn = params.getRawParameterValue("calcCompGain");
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
    
    if (numInputs == 4 && stereoModeIdx->load() < eStereoMode::trueMsIdx)
    {
        params.getParameter("stereoMode")->setValueNotifyingHost(params.getParameter("stereoMode")->convertTo0to1((float) eStereoMode::trueMsIdx));
    }
    else if (numInputs == 2 && stereoModeIdx->load() > eStereoMode::pseudoStereoIdx)
    {
        params.getParameter("stereoMode")->setValueNotifyingHost(params.getParameter("stereoMode")->convertTo0to1((float) eStereoMode::pseudoMsIdx));
    }
    
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
    rotatedEightLeftRightBuffer.setSize(2, currentBlockSize);
    rotatedEightLeftRightBuffer.clear();
    xyLeftRightBuffer.setSize(2, currentBlockSize);
    xyLeftRightBuffer.clear();
    blumleinLeftRightBuffer.setSize(2, currentBlockSize);
    blumleinLeftRightBuffer.clear();
    
    getXyAngleRelatedGains(params.getRawParameterValue("trueStXyAngle")->load());
    previousXyEightRotationGainLeft = currentBlumleinEightRotationGainLeft;
    previousXyEightRotationGainFront = currentXyEightRotationGainFront;
    
    getBlumleinRotationGains(params.getRawParameterValue("blumleinRot")->load());
    previousBlumleinEightRotationGainFront = currentBlumleinEightRotationGainFront;
    previousBlumleinEightRotationGainLeft = currentBlumleinEightRotationGainLeft;
    
    previousMidGain = Decibels::decibelsToGain(params.getRawParameterValue("msMidGain")->load());
    previousSideGain = Decibels::decibelsToGain(params.getRawParameterValue("msSideGain")->load());
    previousPseudoStereoPattern = params.getRawParameterValue("pseudoStPattern")->load();
    previousMsMidPattern = params.getRawParameterValue("msMidPattern")->load();
    previousTrueStereoPattern = params.getRawParameterValue("trueStXyPattern")->load();
    
    blocksToAverage = secondsToAverage * currentSampleRate / currentBlockSize;
}

void StereoCreatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool StereoCreatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if ((layouts.getMainOutputChannelSet() != AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::quadraphonic())
        || (layouts.getMainInputChannelSet() != AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() != AudioChannelSet::quadraphonic()))
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
    
    for (int i = 0; i < buffer.getNumChannels(); ++i)
    {
        inRms[i] = buffer.getRMSLevel (i, 0, numSamples);
    }
    
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
    
    auto currentMidGain = Decibels::decibelsToGain(params.getRawParameterValue("msMidGain")->load());
    auto currentSideGain = Decibels::decibelsToGain(params.getRawParameterValue("msSideGain")->load());
    auto currentPseudoStereoPattern = params.getRawParameterValue("pseudoStPattern")->load();
    
    // one OC-818 was used, ms is calculated with left/right signals
    if (totalNumInputChannels == 2)
    {
        switch ((int) stereoModeIdx->load())
        {
            case eStereoMode::pseudoMsIdx: // when ms is chosen
                // applying mid gain
                applyGainWithRamp(previousMidGain, currentMidGain, &omniEightLrBuffer, 0, numSamples);
                previousMidGain = currentMidGain;
                
                // apply side gain
                applyGainWithRamp(previousSideGain, currentSideGain, &omniEightLrBuffer, 1, numSamples);
                previousSideGain = currentSideGain;
                
                // creating ms left and right patterns
                FloatVectorOperations::copy(writePointerMsLeft, writePointerOmniLR, numSamples);
                FloatVectorOperations::add(writePointerMsLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerMsRight, writePointerOmniLR, numSamples);
                FloatVectorOperations::subtract(writePointerMsRight, writePointerEightLR, numSamples);
                
                buffer.copyFrom(0, 0, msLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, msLeftRightBuffer, 1, 0, numSamples);
                break;
                
            case eStereoMode::pseudoStereoIdx: // when pseudo-stereo is chosen
                // applying pattern weights
                applyGainWithRamp(1.0f - previousPseudoStereoPattern, 1.0f - currentPseudoStereoPattern, &omniEightLrBuffer, 0, numSamples);
                applyGainWithRamp(previousPseudoStereoPattern, currentPseudoStereoPattern, &omniEightLrBuffer, 1, numSamples);
                previousPseudoStereoPattern = currentPseudoStereoPattern;
                
                // merging omni and eight to obtain patterns
                FloatVectorOperations::copy(writePointerPassThroughLeft, writePointerOmniLR, numSamples);
                FloatVectorOperations::add(writePointerPassThroughLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerPassThroughRight, writePointerOmniLR, numSamples);
                FloatVectorOperations::subtract(writePointerPassThroughRight, writePointerEightLR, numSamples);
                
                buffer.copyFrom(0, 0, passThroughLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, passThroughLeftRightBuffer, 1, 0, numSamples);
                break;
                
            default:
                break;
        }

    }
    // two OC-818 were used, ms is calculated from left/right and front/back signals
    else if (totalNumInputChannels == 4)
    {
        // initialising pointer for further calculations
        const float* readPointerFront = buffer.getReadPointer(2);
        const float* readPointerBack = buffer.getReadPointer(3);
        
        float* writePointerMsMid = msMidBuffer.getWritePointer(0);
        
        float* writePointerOmniFB = omniEightFbBuffer.getWritePointer(0);
        float* writePointerEightFB = omniEightFbBuffer.getWritePointer(1);
        
        float* writePointerRotatedEightLeft = rotatedEightLeftRightBuffer.getWritePointer(0);
        float* writePointerRotatedEightRight = rotatedEightLeftRightBuffer.getWritePointer(1);
        
        float* writePointerXyLeft = xyLeftRightBuffer.getWritePointer(0);
        float* writePointerXyRight = xyLeftRightBuffer.getWritePointer(1);
        
        float* writePointerBlumleinLeft = blumleinLeftRightBuffer.getWritePointer(0);
        float* writePointerBlumleinRight = blumleinLeftRightBuffer.getWritePointer(1);
        
        FloatVectorOperations::copy (writePointerOmniFB, readPointerFront, numSamples);
        FloatVectorOperations::add (writePointerOmniFB, readPointerBack, numSamples);
        
        FloatVectorOperations::copy (writePointerEightFB, readPointerFront, numSamples);
        FloatVectorOperations::subtract (writePointerEightFB, readPointerBack, numSamples);
        
        auto currentMsMidPattern = params.getRawParameterValue("msMidPattern")->load();
        auto currentTrueStereoPattern = params.getRawParameterValue("trueStXyPattern")->load();
        
        switch ((int) stereoModeIdx->load())
        {
            case eStereoMode::trueMsIdx:
                FloatVectorOperations::copy(writePointerMsMid, writePointerEightFB, numSamples);
                
                // applying pattern weights
                applyGainWithRamp(previousMsMidPattern, currentMsMidPattern, &msMidBuffer, 0, numSamples);
                applyGainWithRamp(1.0f - previousMsMidPattern, 1.0f - currentMsMidPattern, &omniEightFbBuffer, 0, numSamples);
                previousMsMidPattern = currentMsMidPattern;
                
                // calculating mid pattern
                FloatVectorOperations::add(writePointerMsMid, writePointerOmniFB, numSamples);
                
                // applying mid gain
                applyGainWithRamp(previousMidGain, currentMidGain, &msMidBuffer, 0, numSamples);
                previousMidGain = currentMidGain;
                
                // apply side gain
                applyGainWithRamp(previousSideGain, currentSideGain, &omniEightLrBuffer, 1, numSamples);
                previousSideGain = currentSideGain;
                
                // calculating left and right ms-channel
                FloatVectorOperations::copy(writePointerMsLeft, writePointerMsMid, numSamples);
                FloatVectorOperations::add(writePointerMsLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerMsRight, writePointerMsMid, numSamples);
                FloatVectorOperations::subtract(writePointerMsRight, writePointerEightLR, numSamples);
                
                // writing result to output buffer
                buffer.copyFrom(0, 0, msLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, msLeftRightBuffer, 1, 0, numSamples);
                break;
                
            case eStereoMode::trueStereoIdx:
                
                applyGainWithRamp(previousXyEightRotationGainFront, currentXyEightRotationGainFront, &omniEightFbBuffer, 1, numSamples);
                applyGainWithRamp(previousXyEightRotationGainLeft, currentXyEightRotationGainLeft, &omniEightLrBuffer, 1, numSamples);
                
                FloatVectorOperations::copy(writePointerRotatedEightLeft, writePointerEightFB, numSamples);
                FloatVectorOperations::add(writePointerRotatedEightLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerRotatedEightRight, writePointerEightFB, numSamples);
                FloatVectorOperations::subtract(writePointerRotatedEightRight, writePointerEightLR, numSamples);
                
                applyGainWithRamp(previousTrueStereoPattern, currentTrueStereoPattern, &rotatedEightLeftRightBuffer, 0, numSamples);
                applyGainWithRamp(previousTrueStereoPattern, currentTrueStereoPattern, &rotatedEightLeftRightBuffer, 1, numSamples);
                applyGainWithRamp(1.0f - previousTrueStereoPattern, 1.0f - currentTrueStereoPattern, &omniEightFbBuffer, 0, numSamples);
                previousTrueStereoPattern = currentTrueStereoPattern;
                previousXyEightRotationGainFront = currentXyEightRotationGainFront;
                previousXyEightRotationGainLeft = currentXyEightRotationGainLeft;
                
                FloatVectorOperations::copy(writePointerXyLeft, writePointerRotatedEightLeft, numSamples);
                FloatVectorOperations::add(writePointerXyLeft, writePointerOmniFB, numSamples);
                
                FloatVectorOperations::copy(writePointerXyRight, writePointerRotatedEightRight, numSamples);
                FloatVectorOperations::add(writePointerXyRight, writePointerOmniFB, numSamples);
                
                buffer.copyFrom(0, 0, xyLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, xyLeftRightBuffer, 1, 0, numSamples);
                break;
                
            case eStereoMode::blumleinIdx:
                // preparing left blumlein channel with FB eight
                FloatVectorOperations::copy(writePointerBlumleinLeft, writePointerEightFB, numSamples);
                // preparing right blumlein channel with negative LR eight
                FloatVectorOperations::copyWithMultiply(writePointerBlumleinRight, writePointerEightLR, - 1.0f, numSamples);
                
                // applying FB eight rotation gain for left blumlein channel (copy of FB eight)
                applyGainWithRamp(previousBlumleinEightRotationGainLeft, currentBlumleinEightRotationGainLeft, &blumleinLeftRightBuffer, 0, numSamples);
                // applying LR eight rotation gain for LR eight
                applyGainWithRamp(previousBlumleinEightRotationGainFront, currentBlumleinEightRotationGainFront, &omniEightLrBuffer, 1, numSamples);
                // adding manipulated FB eight (left blumlein Ch) and LR eight (original LR eight)
                FloatVectorOperations::add(writePointerBlumleinLeft, writePointerEightLR, numSamples);
                
                // applying LR eight rotation gain for right blumlein channel (copy of LR eight)
                applyGainWithRamp(previousBlumleinEightRotationGainLeft, currentBlumleinEightRotationGainLeft, &blumleinLeftRightBuffer, 1, numSamples);
                // applying FB eight rotation gain for FB eight
                applyGainWithRamp(previousBlumleinEightRotationGainFront, currentBlumleinEightRotationGainFront, &omniEightFbBuffer, 1, numSamples);
                // adding manipulated LR eight (right blumlein Ch) and FB eight (original FB eight)
                FloatVectorOperations::add(writePointerBlumleinRight, writePointerEightFB, numSamples);
                
                // resetting gains
                previousBlumleinEightRotationGainLeft = currentBlumleinEightRotationGainLeft;
                previousBlumleinEightRotationGainFront = currentBlumleinEightRotationGainFront;
                
                buffer.copyFrom(0, 0, blumleinLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, blumleinLeftRightBuffer, 1, 0, numSamples);
                break;
            default:
                break;
        }
        buffer.clear(2, 0, numSamples);
        buffer.clear(3, 0, numSamples);
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
    
    if (autoLevelsOn->load() >= 0.5f)
    {
        if (counter == blocksToAverage)
        {
            float newOverallGain = (inputGainMean / outGainMean);
            newOverallGain = Decibels::gainToDecibels(newOverallGain);
            
            params.getParameter("compensationGain"+String(stereoModeIdx->load()))->setValueNotifyingHost(params.getParameter("compensationGain"+String(stereoModeIdx->load()))->convertTo0to1(newOverallGain));
            
            params.getParameter("calcCompGain")->setValueNotifyingHost(false);
            inputGainMean = 0.000001f;
            outGainMean = 0.000001f;
            counter = 0;
        }
        else
        {
            inputGainMean += (inRms[0].get() + inRms[1].get()) / 2.0f;
            outGainMean += (outRms[0].get() + outRms[1].get()) / 2.0f / (currentOverallGain + 0.000001f);
            counter++;
        }
    }
    
    currentOverallGain = Decibels::decibelsToGain(params.getRawParameterValue("compensationGain"+String(stereoModeIdx->load()))->load());
    applyGainWithRamp(previousOverallGain, currentOverallGain, &buffer, 0, numSamples);
    applyGainWithRamp(previousOverallGain, currentOverallGain, &buffer, 1, numSamples);
    previousOverallGain = currentOverallGain;
    
    outRms[0] = buffer.getRMSLevel(0, 0, numSamples);
    outRms[1] = buffer.getRMSLevel(1, 0, numSamples);
    
    jassert(buffer.getRMSLevel(0, 0, numSamples) <= 1.1f);
    jassert(buffer.getRMSLevel(1, 0, numSamples) <= 1.1f);
    
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
    if (abLayerState == eCurrentActiveLayer::layerA)
    {
        layerA = params.copyState();
    }
    if (abLayerState == eCurrentActiveLayer::layerB)
    {
        layerB = params.copyState();
    }

    ValueTree vtsState = params.copyState();
    ValueTree AState = layerA.createCopy();
    ValueTree BState = layerB.createCopy();
    
    allValueTreeStates.removeAllChildren(nullptr);
    allValueTreeStates.addChild(vtsState, 0, nullptr);
    allValueTreeStates.addChild(AState, 1, nullptr);
    allValueTreeStates.addChild(BState, 2, nullptr);
    
    std::unique_ptr<XmlElement> xml (allValueTreeStates.createXml());
    copyXmlToBinary (*xml, destData);
}

void StereoCreatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (allValueTreeStates.getType()))
        {
            allValueTreeStates = ValueTree::fromXml (*xmlState);
            params.replaceState(allValueTreeStates.getChild(1));
            layerB = allValueTreeStates.getChild(2).createCopy();
        }
    }
}

void StereoCreatorAudioProcessor::parameterChanged(const String &parameterID, float newValue)
{
    if (parameterID == "trueStXyAngle")
    {
        getXyAngleRelatedGains(params.getRawParameterValue("trueStXyAngle")->load());
    }
    else if (parameterID == "blumleinRot")
    {
        getBlumleinRotationGains(params.getRawParameterValue("blumleinRot")->load());
    }
    else if (parameterID == "stereoMode")
    {
        previousOverallGain = 0.0001f; //currentOverallGain;
        currentOverallGain = Decibels::decibelsToGain(params.getRawParameterValue("compensationGain"+String(stereoModeIdx->load()))->load());
    }
}

void StereoCreatorAudioProcessor::getXyAngleRelatedGains(float currentAngle)
{
    float angle = currentAngle / 2.0f;
    
    currentXyEightRotationGainFront = cos(angle * MathConstants<float>::pi / 180.0f);
    currentXyEightRotationGainLeft = sin(angle * MathConstants<float>::pi / 180.0f);
}

void StereoCreatorAudioProcessor::getBlumleinRotationGains(float currentRotation)
{
    float angle = currentRotation + 45.0f;
    
    currentBlumleinEightRotationGainFront = cos(angle * MathConstants<float>::pi / 180.0f);
    currentBlumleinEightRotationGainLeft = sin(angle * MathConstants<float>::pi / 180.0f);
    
}

void StereoCreatorAudioProcessor::setAbLayer(int desiredLayer)
{
    abLayerState = desiredLayer;
    changeAbLayerState();
}

void StereoCreatorAudioProcessor::changeAbLayerState()
{
    if (abLayerState == eCurrentActiveLayer::layerB)
    {
        layerA = params.copyState();
        params.state = layerB.createCopy();
    }
    else
    {
        layerB = params.copyState();
        params.state = layerA.createCopy();
    }
    
    // in case the number of input channels changed
    if (numInputs == 4 && stereoModeIdx->load() < eStereoMode::trueMsIdx)
    {
        params.getParameter("stereoMode")->setValueNotifyingHost(params.getParameter("stereoMode")->convertTo0to1((float) eStereoMode::trueMsIdx));
    }
    else if (numInputs == 2 && stereoModeIdx->load() > eStereoMode::pseudoStereoIdx)
    {
        params.getParameter("stereoMode")->setValueNotifyingHost(params.getParameter("stereoMode")->convertTo0to1((float) eStereoMode::pseudoMsIdx));
    }
}

void StereoCreatorAudioProcessor::applyGainWithRamp(float previousGain, float currentGain, AudioBuffer<float>* buff, int bufferChannel, int numSamples)
{
    if (previousGain == currentGain)
    {
        buff->applyGain(bufferChannel, 0, numSamples, currentGain);
    }
    else
    {
        buff->applyGainRamp(bufferChannel, 0, numSamples, previousGain, currentGain);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoCreatorAudioProcessor();
}


