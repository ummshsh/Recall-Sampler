#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ColourPalette.cpp"
#include "FlashbackVisualiser.cpp"
#include "DraggableNumberBox.cpp"
#include "CustomLookAndFeel.h"

class NewProjectAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor(NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    bool saveWav(const juce::AudioSampleBuffer& buffer);

    NewProjectAudioProcessor& audioProcessor;
    ColourPalette palette;

    juce::File file;
    juce::AudioFormatManager formatManager;

    juce::DrawableButton freezeButton;
    DraggableNumberBox recordTimeBox;
    FlashbackVisualiser flashbackVisualiser;

    std::unique_ptr<CustomLookAndFeel> customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessorEditor)
};