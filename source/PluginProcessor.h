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

#include <atomic>
#include <cfloat>
#include <juce_audio_processors/juce_audio_processors.h>
#include <math.h>

enum eCurrentActiveLayer
{
    layerA = 1,
    layerB = 2
};

static inline bool doublesEquivalent (double a, double b)
{
    return fabs (a - b) < DBL_EPSILON;
}

static inline bool floatsEquivalent (double a, double b)
{
    return fabs (a - b) < FLT_EPSILON;
}

//==============================================================================
/**
*/
class StereoCreatorAudioProcessor final : public juce::AudioProcessor,
                                          public juce::AudioProcessorValueTreeState::Listener
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

    using juce::AudioProcessor::processBlock;
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
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    int getStereoModeIdx() { return (static_cast<int> (stereoModeIdx->load())); }
    int getNumInpCh() { return numInputs; }
    void getXyAngleRelatedGains (float currentAngle);
    void getBlumleinRotationGains (float currentRotation);
    void changeAbLayerState();
    void setAbLayer (int desiredLayer);

    void applyGainWithRamp (float previousGain,
                            float currentGain,
                            juce::AudioBuffer<float>* buff,
                            int bufferChannel,
                            int numSamples);
    bool compensationGainCalcOver() { return autoLevelsOn->load() > 0.5f; }

    int getAbLayerState() { return abLayerState.load (std::memory_order_relaxed); }

    //    Atomic<bool> wrongBusConfiguration = false;

    std::atomic<float> inRms[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    std::atomic<float> outRms[2] = { 0.0f, 0.0f };

private:
    juce::AudioProcessorValueTreeState params;

    // AB layer handling
    juce::Identifier nodeA = "layerA";
    juce::Identifier nodeB = "layerB";
    juce::Identifier allStates = "savedLayers";
    juce::ValueTree layerA;
    juce::ValueTree layerB;
    juce::ValueTree allValueTreeStates;

    std::atomic<int> abLayerState = eCurrentActiveLayer::layerA;

    int numInputs;

    std::atomic<float>* stereoModeIdx;

    std::atomic<float>* channelSwitchOn;
    std::atomic<float>* autoLevelsOn;

    std::atomic<bool> isPlaying = false;

    juce::AudioBuffer<float> omniEightLrBuffer;
    juce::AudioBuffer<float> omniEightFbBuffer;
    juce::AudioBuffer<float> msMidBuffer;
    juce::AudioBuffer<float> msLeftRightBuffer;
    juce::AudioBuffer<float> chSwitchBuffer;
    juce::AudioBuffer<float> passThroughLeftRightBuffer;
    juce::AudioBuffer<float> rotatedEightLeftRightBuffer;
    juce::AudioBuffer<float> xyLeftRightBuffer;
    juce::AudioBuffer<float> blumleinLeftRightBuffer;

    float currentXyEightRotationGainFront;
    float currentXyEightRotationGainLeft;
    float previousXyEightRotationGainFront;
    float previousXyEightRotationGainLeft;

    float currentBlumleinEightRotationGainFront;
    float currentBlumleinEightRotationGainLeft;
    float previousBlumleinEightRotationGainFront;
    float previousBlumleinEightRotationGainLeft;

    int counter = 0;
    double secondsToAverage = 1.5;
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
