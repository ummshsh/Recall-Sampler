#pragma once

#include <JuceHeader.h>

class NewProjectAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    const juce::AudioBuffer<float>& getRecording();
    void setFrozen(bool shouldBeFrozen);
    void setRecordingDuration(double newDurationInSeconds);
    void applyRecordingDurationChange();
    float getRecordingDuration() const; 

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    std::unique_ptr<juce::AudioBuffer<float>> flashbackBuffer;
    juce::int64 currentBufferPostion;

    std::atomic<bool> isFrozen;
    std::atomic<float> recordingDurationSecs;

private:
    std::atomic<bool> isPausedBySilence;
    float silenceDurationSeconds;
    const float silenceThresholdSeconds = 3.0f;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)
};
