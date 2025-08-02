#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DraggableNumberBox.cpp"
#include "FlashbackVisualiser.cpp"

class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    bool saveWav(const juce::AudioSampleBuffer& buffer);
    ~NewProjectAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
private:
    NewProjectAudioProcessor& audioProcessor;
    juce::AudioFormatManager formatManager;
    juce::File file;

    ColourPalette palette;

    FlashbackVisualiser flashbackVisualiser;

    DraggableNumberBox recordTimeBox;
    juce::DrawableButton freezeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
