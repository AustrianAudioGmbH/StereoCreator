/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

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
    
//    Atomic<bool> wrongBusConfiguration = false;
    
    Atomic<float> inRms[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Atomic<float> outRms[2] = { 0.0f, 0.0f};
    
private:
    AudioProcessorValueTreeState params;
    
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
    
    float xyEightRotationGainFront = 0.7f;
    float xyEightRotationGainLeft = 0.7f;
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
