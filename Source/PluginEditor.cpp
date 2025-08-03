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

    setSize(720, 250);
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

    juce::String snowflakeSVG = R"(
        <svg viewBox="0 0 50 50" version="1.2" baseProfile="tiny" xmlns="http://www.w3.org/2000/svg" overflow="inherit">
            <path d="M23 47.689v-6.342l-3.357 1.992-1.643-1.034v-2.229l5-2.986v-4.168l-4 2.451v-4.416l-4 2.094v5.99l-1.653 1.23-1.347-1.111v-4.012l-5.574 3.122-2.426-.999v-2.529l5.685-3.17-3.685-1.822v-2.32l2.123-1.127 5.214 3.068 3.612-2.084-.082-.065-3.665-2.123 3.568-2.228-3.577-2.083-5.213 3.052-1.98-.969v-2.307l3.542-1.978-5.542-3.053v-2.529l2.321-1.114 5.679 3.197v-4.076l1.485-1.127 1.943 1.18-.056 6.105 3.673 2.122.033-4.311 3.922 2.099v-4.167l-5-2.988v-2.214l1.643-1.05 3.357 1.992v-6.328l1.994-1.324 2.006 1.324v6.328l3.906-2.031 2.094 1.219v1.992l-6 3.08v4.167l4-2.267v4.534l4-2.084v-6.524l1.455-.866 1.545.865v4.167l5.842-3.08 2.158 1.218v2.359l-5.495 3.17 3.495 1.954v2.254l-1.83.996-5.327-3.158-3.679 2.346 3.549 2.228-3.659 2.122 3.772 1.992 5.389-2.986 1.785 1.216v2.15l-3.32 1.887 5.32 3.17v2.49l-2.522 1.037-5.478-2.988v3.955l-1.52 1.049-1.48-1.049v-6.002l-4-2.213v4.168l-4-2.268v4.168l5 2.986v2.359l-1.647.904-3.353-1.99v6.342l-2.006 1.31-1.994-1.311zm-1.466-22.597l1.886 2.908h3.514l1.613-2.908-1.704-3.092h-3.514l-1.795 3.092z"/>
        </svg>
    )";

    std::unique_ptr<juce::Drawable> snowflakeIconOff = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(snowflakeSVG));
    std::unique_ptr<juce::Drawable> snowflakeIconOn = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(snowflakeSVG));

    snowflakeIconOff->replaceColour(juce::Colours::black, palette.freezeButtonOff);
    snowflakeIconOn->replaceColour(juce::Colours::black, juce::Colours::white);

    freezeButton.setImages(snowflakeIconOff.get(), nullptr, nullptr, nullptr, snowflakeIconOn.get(), nullptr, nullptr, nullptr);
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
    const int padding = 20;
    const int headerHeight = 50;

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