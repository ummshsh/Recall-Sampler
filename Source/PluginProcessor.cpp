#include "PluginProcessor.h"
#include "PluginEditor.h"

NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    isFrozen = false;
    recordingDurationSecs = 30.0f;
    isPausedBySilence = false;
    silenceDurationSeconds = 0.0f;
    currentBufferPostion = 0;
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

void NewProjectAudioProcessor::setFrozen(bool shouldBeFrozen)
{
    isFrozen.store(shouldBeFrozen);
}

float NewProjectAudioProcessor::getRecordingDuration() const
{
    return recordingDurationSecs.load();
}

void NewProjectAudioProcessor::setRecordingDuration(double newDurationInSeconds)
{
    recordingDurationSecs.store(static_cast<float>(newDurationInSeconds));
}

void NewProjectAudioProcessor::applyRecordingDurationChange()
{
    suspendProcessing(true);

    const auto newDuration = recordingDurationSecs.load();
    const int requiredSamples = static_cast<int>(newDuration * getSampleRate());

    if (flashbackBuffer->getNumSamples() != requiredSamples)
    {
        DBG("Applying buffer resize. New duration: " + juce::String(newDuration) + "s");
        flashbackBuffer->setSize(getTotalNumInputChannels(), requiredSamples, true, true, true);
        flashbackBuffer->clear();
        currentBufferPostion = 0;
    }

    suspendProcessing(false);
}

const juce::AudioBuffer<float>& NewProjectAudioProcessor::getRecording()
{
    return *flashbackBuffer;
}

const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NewProjectAudioProcessor::getNumPrograms() { return 1; }
int NewProjectAudioProcessor::getCurrentProgram() { return 0; }
void NewProjectAudioProcessor::setCurrentProgram(int index) {}
const juce::String NewProjectAudioProcessor::getProgramName(int index) { return {}; }
void NewProjectAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const float initialDuration = recordingDurationSecs.load();
    currentBufferPostion = 0;

    flashbackBuffer = std::make_unique<juce::AudioBuffer<float>>(getTotalNumInputChannels(), sampleRate * initialDuration);
    flashbackBuffer->clear();

    isPausedBySilence.store(false);
    silenceDurationSeconds = 0.0f;
}


void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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
void NewProjectAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (getSampleRate() <= 0)
        return;

    if (isFrozen.load())
        return;

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();

    const float bufferDuration = (float)buffer.getNumSamples() / getSampleRate();
    if (buffer.getMagnitude(0, buffer.getNumSamples()) < 0.0002f)
    {
        silenceDurationSeconds += bufferDuration;
        if (silenceDurationSeconds >= silenceThresholdSeconds && !isPausedBySilence.load())
        {
            isPausedBySilence.store(true);
            DBG("Silence detected for 3 seconds. PAUSING recording.");
        }
    }
    else
    {
        silenceDurationSeconds = 0.0f;
        isPausedBySilence.store(false);
    }

    if (!isPausedBySilence.load())
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getReadPointer(channel);
            int remainingSpace = flashbackBuffer->getNumSamples() - currentBufferPostion;
            if (buffer.getNumSamples() <= remainingSpace)
            {
                flashbackBuffer->copyFrom(channel, currentBufferPostion, channelData, buffer.getNumSamples());
            }
            else
            {
                flashbackBuffer->copyFrom(channel, currentBufferPostion, channelData, remainingSpace);
                flashbackBuffer->copyFrom(channel, 0, channelData + remainingSpace, buffer.getNumSamples() - remainingSpace);
            }
        }
        currentBufferPostion = (currentBufferPostion + buffer.getNumSamples()) % flashbackBuffer->getNumSamples();
    }
}

bool NewProjectAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor(*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
