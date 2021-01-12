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
    std::make_unique<AudioParameterInt> ("stereoMode", "Stereo Mode", 1, 5, 1, "",
                                           [](int value, int maximumStringLength) {return String(value + 1);}, nullptr),
    std::make_unique<AudioParameterFloat> ("msMidGain", "MS Mid Gain", NormalisableRange<float>( - 32.0f, 3.0f, 0.1f), -6.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("msSideGain", "MS Side Gain", NormalisableRange<float>( - 32.0f, 3.0f, 0.1f), -6.0f,  "dB", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("lrWidth", "LR Width", NormalisableRange<float>( 0.0f, 100.0f, 1.0f), 70.0f,  "%", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
    std::make_unique<AudioParameterBool>("channelSwitch", "Channel Switch", false, "", [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
    std::make_unique<AudioParameterBool>("autoLevelMode", "Auto Level Mode", false, "", [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
    std::make_unique<AudioParameterFloat> ("msMidPattern", "MS Mid Pattern", NormalisableRange<float> (0.0f, 0.75f, 0.01f), 0.0f, "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
    std::make_unique<AudioParameterFloat> ("trueStXyPattern", "True-Stereo XY Pattern", NormalisableRange<float> (0.37f, 0.75f, 0.01f), 0.5f, "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 2); }, nullptr),
    std::make_unique<AudioParameterFloat> ("trueStXyAngle", "True-Stereo XY Angle", NormalisableRange<float> (30.0f, 150.0f, 0.5f), 90.0f, "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr),
    std::make_unique<AudioParameterFloat> ("blumleinRot", "Blumlein Rotation", NormalisableRange<float> (- 30.0f, 30.0f, 0.5f), 0.0f, "", AudioProcessorParameter::genericParameter, [](float value, int maximumStringLength) { return String(value, 1); }, nullptr)
}),
layerA(nodeA), layerB(nodeB), allValueTreeStates(allStates)
{
    params.addParameterListener("stereoMode", this);
    params.addParameterListener("msMidGain", this);
    params.addParameterListener("msSideGain", this);
    params.addParameterListener("channelSwitch", this);
    params.addParameterListener("autoLevelMode", this);
    params.addParameterListener("msMidPattern", this);
    params.addParameterListener("trueStXyPattern", this);
    params.addParameterListener("trueStXyAngle", this);
    params.addParameterListener("blumleinRot", this);
    
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
    rotatedEightLeftRightBuffer.setSize(2, currentBlockSize);
    rotatedEightLeftRightBuffer.clear();
    xyLeftRightBuffer.setSize(2, currentBlockSize);
    xyLeftRightBuffer.clear();
    blumleinLeftRightBuffer.setSize(2, currentBlockSize);
    blumleinLeftRightBuffer.clear();
    
    for (int i = 0; i < panTableSize; i++)
    {
        panTableLeft[i] = std::powf(cos(MathConstants<float>::pi / 2.0f * ((float) i / ((float) panTableSize))), panLawExp);
        panTableRight[i] = std::powf(sin(MathConstants<float>::pi / 2.0f * ((float) i / ((float) panTableSize - 1.0f))), panLawExp);
//        panTableLeft[i] = cos((float) i / ((float) panTableSize));
//        panTableRight[i] = sin((float) i / ((float) panTableSize - 1.0f));
    }
    
    for (int i = 0; i < xyAnglePanTableSize; i++)
    {
        xyAnglePanTableLeft[i] = panTableRight[i + 2501];
        xyAnglePanTableFront[i] = panTableLeft[i + 2501];
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
    
    float eightPatternMultiplier = params.getParameter("lrWidth")->convertTo0to1(params.getRawParameterValue("lrWidth")->load()) * hyperCardioidLimit; // maximum is hyper cardioid => 0.75
    float omniPatternMultiplier = 1.0f - eightPatternMultiplier;
    
    FloatVectorOperations::copy (writePointerOmniLR, readPointerLeft, numSamples);
    FloatVectorOperations::add (writePointerOmniLR, readPointerRight, numSamples);
    
    FloatVectorOperations::copy (writePointerEightLR, readPointerLeft, numSamples);
    FloatVectorOperations::subtract (writePointerEightLR, readPointerRight, numSamples);
    
    // one OC-818 was used, ms is calculated with left/right signals
    if (totalNumInputChannels == 2)
    {
        switch ((int) stereoModeIdx->load())
        {
            case eStereoMode::pseudoMsIdx: // when ms is chosen
                // creating ms left and right patterns
                FloatVectorOperations::multiply(writePointerOmniLR, Decibels::decibelsToGain(params.getRawParameterValue("msMidGain")->load()), numSamples);
                FloatVectorOperations::multiply(writePointerEightLR, Decibels::decibelsToGain(params.getRawParameterValue("msSideGain")->load()), numSamples);
                FloatVectorOperations::copy(writePointerMsLeft, writePointerOmniLR, numSamples);
                FloatVectorOperations::add(writePointerMsLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerMsRight, writePointerOmniLR, numSamples);
                FloatVectorOperations::subtract(writePointerMsRight, writePointerEightLR, numSamples);
                
                buffer.copyFrom(0, 0, msLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, msLeftRightBuffer, 1, 0, numSamples);
                break;
            case eStereoMode::pseudoStereoIdx: // when pass through is chosen
                // adding up omni and eight according to the width setting
                FloatVectorOperations::multiply(writePointerEightLR, eightPatternMultiplier, numSamples);
                FloatVectorOperations::multiply(writePointerOmniLR, omniPatternMultiplier, numSamples);
                
                FloatVectorOperations::copy(writePointerPassThroughLeft, writePointerOmniLR, numSamples);
                FloatVectorOperations::add(writePointerPassThroughLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerPassThroughRight, writePointerOmniLR, numSamples);
                FloatVectorOperations::subtract(writePointerPassThroughRight, writePointerEightLR, numSamples);
                
                buffer.copyFrom(0, 0, passThroughLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, passThroughLeftRightBuffer, 1, 0, numSamples);
                
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
        
        switch ((int) stereoModeIdx->load())
        {
            case eStereoMode::trueMsIdx:
                FloatVectorOperations::copy(writePointerMsMid, writePointerEightFB, numSamples);
                FloatVectorOperations::multiply(writePointerMsMid, params.getRawParameterValue("msMidPattern")->load(), numSamples);
                FloatVectorOperations::addWithMultiply(writePointerMsMid, writePointerOmniFB, 1.0f - params.getRawParameterValue("msMidPattern")->load(), numSamples);
                
                FloatVectorOperations::multiply(writePointerMsMid, Decibels::decibelsToGain(params.getRawParameterValue("msMidGain")->load()), numSamples);
                FloatVectorOperations::multiply(writePointerEightLR, Decibels::decibelsToGain(params.getRawParameterValue("msSideGain")->load()), numSamples);
                
                FloatVectorOperations::copy(writePointerMsLeft, writePointerMsMid, numSamples);
                FloatVectorOperations::add(writePointerMsLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerMsRight, writePointerMsMid, numSamples);
                FloatVectorOperations::subtract(writePointerMsRight, writePointerEightLR, numSamples);

                buffer.copyFrom(0, 0, msLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, msLeftRightBuffer, 1, 0, numSamples);
                break;
                
            case eStereoMode::trueStereoIdx:
                FloatVectorOperations::multiply(writePointerEightFB, xyEightRotationGainFront, numSamples);
                FloatVectorOperations::multiply(writePointerEightLR, xyEightRotationGainLeft, numSamples);
                
                FloatVectorOperations::copy(writePointerRotatedEightLeft, writePointerEightFB, numSamples);
                FloatVectorOperations::add(writePointerRotatedEightLeft, writePointerEightLR, numSamples);
                
                FloatVectorOperations::copy(writePointerRotatedEightRight, writePointerEightFB, numSamples);
                FloatVectorOperations::subtract(writePointerRotatedEightRight, writePointerEightLR, numSamples);
                
                FloatVectorOperations::multiply(writePointerRotatedEightLeft, params.getRawParameterValue("trueStXyPattern")->load(), numSamples);
                FloatVectorOperations::multiply(writePointerRotatedEightRight, params.getRawParameterValue("trueStXyPattern")->load(), numSamples);
                FloatVectorOperations::multiply(writePointerOmniFB, 1.0f - params.getRawParameterValue("trueStXyPattern")->load(), numSamples);
                
                FloatVectorOperations::copy(writePointerXyLeft, writePointerRotatedEightLeft, numSamples);
                FloatVectorOperations::add(writePointerXyLeft, writePointerOmniFB, numSamples);
                
                FloatVectorOperations::copy(writePointerXyRight, writePointerRotatedEightRight, numSamples);
                FloatVectorOperations::add(writePointerXyRight, writePointerOmniFB, numSamples);
                
                buffer.copyFrom(0, 0, xyLeftRightBuffer, 0, 0, numSamples);
                buffer.copyFrom(1, 0, xyLeftRightBuffer, 1, 0, numSamples);
                break;
                
            case eStereoMode::blumleinIdx:
                FloatVectorOperations::copyWithMultiply(writePointerBlumleinLeft, writePointerEightFB, blumleinEightRotationGainFront, numSamples);
                FloatVectorOperations::addWithMultiply(writePointerBlumleinLeft, writePointerEightLR, blumleinEightRotationGainLeft, numSamples);
                
                FloatVectorOperations::copyWithMultiply(writePointerBlumleinRight, writePointerEightFB, blumleinEightRotationGainLeft, numSamples);
                FloatVectorOperations::subtractWithMultiply(writePointerBlumleinRight, writePointerEightLR, blumleinEightRotationGainFront, numSamples);
                
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
    
    outRms[0] = buffer.getRMSLevel(0, 0, numSamples);
    outRms[1] = buffer.getRMSLevel(1, 0, numSamples);
    
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
        }
    }
    layerB = allValueTreeStates.getChild(2).createCopy();
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
    else if (parameterID == "trueStXyAngle")
    {
        getXyAngleRelatedGains(params.getRawParameterValue("trueStXyAngle")->load());
    }
    else if (parameterID == "blumleinRot")
    {
        getBlumleinRotationGains(params.getRawParameterValue("blumleinRot")->load());
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


void StereoCreatorAudioProcessor::getXyAngleRelatedGains(float currentAngle)
{
    float angleRange = params.getParameter("trueStXyAngle")->getNormalisableRange().getRange().getLength() / 2.0f;
    currentAngle /= 2.0f;
    currentAngle = (currentAngle - (params.getParameter("trueStXyAngle")->getNormalisableRange().getRange().getStart() / 2)) / angleRange * xyAnglePanTableSize;
    
    xyEightRotationGainLeft = xyAnglePanTableLeft[(int) currentAngle - 1];
    xyEightRotationGainFront = xyAnglePanTableFront[(int) currentAngle - 1];
}

void StereoCreatorAudioProcessor::getBlumleinRotationGains(float currentRotation)
{
    float rotationOffset = 45.0f;
    currentRotation += rotationOffset;
    float rotationRange = params.getParameter("blumleinRot")->getNormalisableRange().getRange().getLength();
    currentRotation = (currentRotation - (params.getParameter("blumleinRot")->getNormalisableRange().getRange().getEnd() / 2)) / rotationRange * xyAnglePanTableSize;
    
    blumleinEightRotationGainLeft = xyAnglePanTableLeft[(int) currentRotation - 1];
    blumleinEightRotationGainFront = xyAnglePanTableFront[(int) currentRotation - 1];

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
}
