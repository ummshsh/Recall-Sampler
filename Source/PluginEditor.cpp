#include "PluginProcessor.h"
#include "PluginEditor.h"

NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor(NewProjectAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    formatManager(),
    freezeButton("freezeButton", juce::DrawableButton::ButtonStyle::ImageFitted),
    flashbackVisualiser(p, palette),
    recordTimeBox(palette)
{
    customLookAndFeel = std::make_unique<CustomLookAndFeel>(palette);
    setLookAndFeel(customLookAndFeel.get());

    setSize(900, 250);
    addAndMakeVisible(flashbackVisualiser);
    addAndMakeVisible(recordTimeBox);
    addAndMakeVisible(freezeButton);

    flashbackVisualiser.onSelectionDragged = [this, &p](juce::Range<juce::int64> sampleRange)
    {
        p.suspendProcessing(true);
        const auto& fullRecording = p.getRecording();

        juce::AudioBuffer<float> selectedRegion(fullRecording.getNumChannels(), (int)sampleRange.getLength());

        for (int i = 0; i < fullRecording.getNumChannels(); ++i)
        {
            selectedRegion.copyFrom(i, 0, fullRecording, i, (int)sampleRange.getStart(), (int)sampleRange.getLength());
        }
        p.suspendProcessing(false);

        const bool success = saveWav(selectedRegion);

        if (success)
        {
            //DBG("Dragging selection: " + juce::String(sampleRange.getLength()) + " samples");
            flashbackVisualiser.performExternalDragDropOfFiles({ file.getFullPathName() }, false);
        }
    };

    flashbackVisualiser.onFullDragRequested = [this, &p]()
    {
        p.suspendProcessing(true);
        const auto& rec = p.getRecording();
        p.suspendProcessing(false);

        const bool success = saveWav(rec);

        if (success)
        {
            DBG("Dragging full buffer: " + juce::String(rec.getNumSamples()) + " samples");
            flashbackVisualiser.performExternalDragDropOfFiles({ file.getFullPathName() }, false);
        }
    };

    freezeButton.setLookAndFeel(customLookAndFeel.get());

    juce::String freezeSVG = R"(
        <svg fill="#000000" width="800px" height="800px" viewBox="0 0 36 36" version="1.1"  preserveAspectRatio="xMidYMid meet" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
    <title>pause-solid</title>
    <rect class="clr-i-solid clr-i-solid-path-1" x="3.95" y="4" width="11" height="28" rx="2.07" ry="2.07"></rect><rect class="clr-i-solid clr-i-solid-path-2" x="20.95" y="4" width="11" height="28" rx="2.07" ry="2.07"></rect>
    <rect x="0" y="0" width="36" height="36" fill-opacity="0"/>
</svg>
    )";

    std::unique_ptr<juce::Drawable> freezeIconOff = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(freezeSVG));
    std::unique_ptr<juce::Drawable> freezeIconOn = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(freezeSVG));

    freezeIconOff->replaceColour(juce::Colours::black, palette.freezeButtonOff);
    freezeIconOn->replaceColour(juce::Colours::black, palette.freezeButtonOn);

    freezeButton.setImages(freezeIconOff.get(), nullptr, nullptr, nullptr, freezeIconOn.get(), nullptr, nullptr, nullptr);
    freezeButton.setClickingTogglesState(true);

    freezeButton.setClickingTogglesState(true);
    freezeButton.onClick = [this]() {
        audioProcessor.setFrozen(freezeButton.getToggleState());
    };

    recordTimeBox.setValue(audioProcessor.getRecordingDuration(), juce::dontSendNotification);
    recordTimeBox.onValueChanged = [this, &p](double newValue)
    {
        audioProcessor.setRecordingDuration(newValue);
    };

    recordTimeBox.onDragEnd = [this](double /*finalValue*/)
    {
        audioProcessor.applyRecordingDurationChange();
    };
}

bool NewProjectAudioProcessorEditor::saveWav(const juce::AudioSampleBuffer& buffer)
{
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
    {
        //DBG("Empty buffer received, not saving.");
        return false;
    }

    file = juce::File::createTempFile(".wav");

    juce::WavAudioFormat format;
    std::unique_ptr<juce::FileOutputStream> fileStream(file.createOutputStream());

    if (!fileStream)
    {
        //DBG("Failed to create file stream for temp file");
        return false;
    }

    std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(
        fileStream.release(),
        audioProcessor.getSampleRate(),
        buffer.getNumChannels(),
        24,
        {},
        0
    ));

    if (!writer)
    {
        //DBG("Failed to create WAV writer for temp file");
        return false;
    }

    if (!writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples()))
    {
        //DBG("Failed to write audio data to temp file");
        return false;
    }

    //DBG("Successfully saved to temp file: " + file.getFullPathName());
    return true;
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    freezeButton.setLookAndFeel(nullptr);
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(palette.appBackground);
}

void NewProjectAudioProcessorEditor::resized()
{
    const int padding = 10;
    const int headerHeight = 40;

    juce::Rectangle<int> bounds = getLocalBounds().reduced(padding);

    juce::Rectangle<int> headerArea = bounds.removeFromTop(headerHeight);
    bounds.removeFromTop(padding / 2);

    flashbackVisualiser.setBounds(bounds);

    const int buttonSize = 30;
    freezeButton.setBounds(headerArea.removeFromLeft(buttonSize).withSizeKeepingCentre(buttonSize, buttonSize));

    headerArea.removeFromLeft(padding / 2);

    const int numberBoxWidth = 85;
    recordTimeBox.setBounds(headerArea.removeFromLeft(numberBoxWidth));
}