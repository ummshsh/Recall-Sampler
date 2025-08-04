#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ColourPalette.cpp"

class FlashbackVisualiser : public juce::Component,
    public juce::DragAndDropContainer,
    private juce::Timer
{
public:
    FlashbackVisualiser(NewProjectAudioProcessor& p, const ColourPalette& pal) : audioProcessor(p), palette(pal)
    {
        startTimerHz(25);
    }

    ~FlashbackVisualiser() override { stopTimer(); }

    std::function<void()> onFullDragRequested;
    std::function<void(juce::Range<juce::int64> selectedSampleRange)> onSelectionDragged;

    void mouseDown(const juce::MouseEvent& event) override
    {
        isMakingNewSelection = !selectionArea.contains(event.getPosition());
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (isMakingNewSelection)
        {
            const int startX = event.getMouseDownPosition().getX();
            const int endX = event.getPosition().getX();
            const int left = std::min(startX, endX);
            const int right = std::max(startX, endX);
            selectionArea = juce::Rectangle<int>(left, 0, right - left, getHeight());
            repaint();
        }
        else
        {
            if (!selectionArea.isEmpty() && onSelectionDragged)
            {
                onSelectionDragged(convertPixelAreaToSampleRange(selectionArea));
            }
            else if (onFullDragRequested)
            {
                onFullDragRequested();
            }
        }
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        if (!event.mouseWasDraggedSinceMouseDown())
        {
            selectionArea = {};
            repaint();
        }
        isMakingNewSelection = false;
    }

    void paint(juce::Graphics& g) override
    {
        const float cornerRadius = 18.0f;
        auto bounds = getLocalBounds().toFloat();

        g.setColour(palette.visBackground);
        g.fillRoundedRectangle(bounds, cornerRadius);

        g.setColour(palette.controlBorder);
        //g.drawRoundedRectangle(bounds, cornerRadius, 2.f);

        //juce::Path clipPath;
        //clipPath.addRoundedRectangle(bounds.reduced(1.0f), cornerRadius);
        //g.reduceClipRegion(clipPath);

        auto& buffer = *audioProcessor.flashbackBuffer;
        auto numSamples = buffer.getNumSamples();
        if (numSamples == 0) return;

        juce::Path waveformPath;
        const float componentWidth = (float)getWidth();
        const float componentHeight = (float)getHeight();
        const float centerY = componentHeight / 2.0f;
        const auto samplesPerPixel = (float)numSamples / componentWidth;

        auto* leftChannelData = buffer.getReadPointer(0);
        auto* rightChannelData = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : nullptr;

        std::vector<float> minValues;
        minValues.reserve(getWidth());
        waveformPath.startNewSubPath(0, centerY);

        for (int pixelX = 0; pixelX < getWidth(); ++pixelX)
        {
            const auto startSample = (int)(pixelX * samplesPerPixel);
            const auto endSample = (int)((pixelX + 1) * samplesPerPixel);

            float minVal = 0.0f;
            float maxVal = 0.0f;

            if (startSample < endSample)
            {
                auto numSamplesInRange = endSample - startSample;
                auto leftRange = juce::FloatVectorOperations::findMinAndMax(leftChannelData + startSample, numSamplesInRange);

                if (rightChannelData != nullptr)
                {
                    auto rightRange = juce::FloatVectorOperations::findMinAndMax(rightChannelData + startSample, numSamplesInRange);
                    minVal = std::min(leftRange.getStart(), rightRange.getStart());
                    maxVal = std::max(leftRange.getEnd(), rightRange.getEnd());
                }
                else
                {
                    minVal = leftRange.getStart();
                    maxVal = leftRange.getEnd();
                }
            }

            minValues.push_back(minVal);
            const float topY = juce::jmap(maxVal, -1.0f, 1.0f, componentHeight, 0.0f);
            waveformPath.lineTo((float)pixelX, topY);
        }

        for (int pixelX = getWidth() - 1; pixelX >= 0; --pixelX)
        {
            const float bottomY = juce::jmap(minValues[pixelX], -1.0f, 1.0f, componentHeight, 0.0f);
            waveformPath.lineTo((float)pixelX, bottomY);
        }

        waveformPath.closeSubPath();
        g.setColour(palette.visWaveformBody);
        g.fillPath(waveformPath);

        g.setColour(palette.visWaveformOutline);
        g.strokePath(waveformPath, juce::PathStrokeType(1.f));

        if (!selectionArea.isEmpty())
        {
            g.setColour(palette.visSelection);
            g.fillRect(selectionArea);
        }

        const auto writePosition = audioProcessor.currentBufferPostion;
        const float cursorX = ((float)writePosition / (float)numSamples) * componentWidth;

        if (cursorX > 0)
        {
            const int trailWidth = 20;
            const float conceptualTrailStartX = cursorX - trailWidth;
            juce::ColourGradient gradient(palette.visCursor.withAlpha(0.0f),
                conceptualTrailStartX, 0.0f,
                palette.visCursor.withAlpha(0.5f),
                cursorX, 0.0f,
                false);

            const float visibleTrailStartX = std::max(0.0f, conceptualTrailStartX);
            const float visibleTrailWidth = cursorX - visibleTrailStartX;

            g.setGradientFill(gradient);
            g.fillRect(visibleTrailStartX, 0.0f, visibleTrailWidth, componentHeight);
        }

        g.setColour(palette.visCursor);
        g.drawVerticalLine((int)cursorX, 0.0f, (float)getHeight());
    }

private:
    void timerCallback() override { repaint(); }

    juce::Range<juce::int64> convertPixelAreaToSampleRange(juce::Rectangle<int> pixelArea)
    {
        auto clippedPixelArea = pixelArea.getIntersection(getLocalBounds());

        auto numSamples = (juce::int64)audioProcessor.flashbackBuffer->getNumSamples();
        auto componentWidth = (float)getWidth();

        auto startSample = (juce::int64)(clippedPixelArea.getX() * numSamples / componentWidth);
        auto endSample = (juce::int64)(clippedPixelArea.getRight() * numSamples / componentWidth);

        return { startSample, endSample };
    }

    NewProjectAudioProcessor& audioProcessor;
    const ColourPalette& palette;

    juce::Rectangle<int> selectionArea;
    bool isMakingNewSelection = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlashbackVisualiser)
};