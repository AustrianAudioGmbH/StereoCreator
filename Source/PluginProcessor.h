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

    int getStereoModeIdx() { return static_cast<int> (stereoModeIdx->load()); }
    int getNumInpCh() { return numInputs; }
    void getMidGain (float sideGain);
    void getSideGain (float midGain);
    void getXyAngleRelatedGains(float currentAngle);
    void getBlumleinRotationGains (float currentRotation);
    void changeAbLayerState();
    void setAbLayer(int desiredLayer);
    
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
    
    AudioBuffer<float> omniEightLrBuffer; 
    AudioBuffer<float> omniEightFbBuffer;
    AudioBuffer<float> msMidBuffer;
    AudioBuffer<float> msLeftRightBuffer;
    AudioBuffer<float> chSwitchBuffer;
    AudioBuffer<float> passThroughLeftRightBuffer;
    AudioBuffer<float> rotatedEightLeftRightBuffer;
    AudioBuffer<float> xyLeftRightBuffer;
    AudioBuffer<float> blumleinLeftRightBuffer;

    // pan law
    const static int panTableSize = 10000;
    const static int xyAnglePanTableSize = 4996;
    float panLawExp = 4.5f / 3.01f;
    float panTableLeft[panTableSize];
    float panTableRight[panTableSize];
    float xyAnglePanTableFront[xyAnglePanTableSize];
    float xyAnglePanTableLeft[xyAnglePanTableSize];
    
    float xyEightRotationGainFront = 0.707f;
    float xyEightRotationGainLeft = 0.707f;
    float blumleinEightRotationGainFront = 0.7f;
    float blumleinEightRotationGainLeft = 0.7f;
    
    float hyperCardioidLimit = 0.75;
    float wideCardioidLimit = 0.37;
    
    bool midGainChanged = false;
    bool sideGainChanged = false;
    
    
    
    int currentBlockSize;
    double currentSampleRate;
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoCreatorAudioProcessor)
};
