/*
 ==============================================================================
 PluginProcessor.h
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

#pragma once

#include <JuceHeader.h>

enum eStereoMode
{
    pseudoMsIdx = 1,
    pseudoStereoIdx = 2,
    trueMsIdx = 3,
    trueStereoIdx = 4,
    blumleinIdx = 5
};

enum eCurrentActiveLayer
{
    layerA = 1,
    layerB = 2
};

//==============================================================================
/**
*/
class StereoCreatorAudioProcessor  : public juce::AudioProcessor, public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    StereoCreatorAudioProcessor();
    ~StereoCreatorAudioProcessor() override;

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
    
    //==============================================================================
    void parameterChanged (const String &parameterID, float newValue) override;

    int getStereoModeIdx() { return  (stereoModeIdx->load()); }
    int getNumInpCh() { return numInputs; }
    void getXyAngleRelatedGains(float currentAngle);
    void getBlumleinRotationGains (float currentRotation);
    void changeAbLayerState();
    void setAbLayer(int desiredLayer);
    
    void applyGainWithRamp (float previousGain, float currentGain, AudioBuffer<float>* buff, int bufferChannel, int numSamples);
    bool compensationGainCalcOver() { return autoLevelsOn->load() > 0.5f; }
    
//    Atomic<bool> wrongBusConfiguration = false;
    
    Atomic<float> inRms[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Atomic<float> outRms[2] = { 0.0f, 0.0f};
    
private:
    AudioProcessorValueTreeState params;
    
    // AB layer handling
    Identifier nodeA = "layerA";
    Identifier nodeB = "layerB";
    Identifier allStates = "savedLayers";
    ValueTree layerA;
    ValueTree layerB;
    ValueTree allValueTreeStates;

    int abLayerState;
    
    int numInputs;
    
    std::atomic<float>* stereoModeIdx;
    
    std::atomic<float>* channelSwitchOn;
    std::atomic<float>* autoLevelsOn;
    
    Atomic<bool> isPlaying = false;
    
    AudioBuffer<float> omniEightLrBuffer; 
    AudioBuffer<float> omniEightFbBuffer;
    AudioBuffer<float> msMidBuffer;
    AudioBuffer<float> msLeftRightBuffer;
    AudioBuffer<float> chSwitchBuffer;
    AudioBuffer<float> passThroughLeftRightBuffer;
    AudioBuffer<float> rotatedEightLeftRightBuffer;
    AudioBuffer<float> xyLeftRightBuffer;
    AudioBuffer<float> blumleinLeftRightBuffer;
    
    float currentXyEightRotationGainFront;
    float currentXyEightRotationGainLeft;
    float previousXyEightRotationGainFront;
    float previousXyEightRotationGainLeft;
    
    float currentBlumleinEightRotationGainFront;
    float currentBlumleinEightRotationGainLeft;
    float previousBlumleinEightRotationGainFront;
    float previousBlumleinEightRotationGainLeft;
    
    float previousCompensationGain[5];
    int counter = 0;
    float secondsToAverage = 1.5f;
    int blocksToAverage;
    float inputGainMean = 0.000001f;
    float outGainMean = 0.000001f;
    
    float previousMidGain;
    float previousSideGain;
    float previousPseudoStereoPattern;
    float previousMsMidPattern;
    float previousTrueStereoPattern;
    float previousOverallGain;
    
    float currentOverallGain;
    
    int currentBlockSize;
    double currentSampleRate;
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoCreatorAudioProcessor)
};
