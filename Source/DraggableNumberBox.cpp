#pragma once

#include <JuceHeader.h>
#include "ColourPalette.cpp"

class DraggableNumberBox : public juce::Component
{
public:
    DraggableNumberBox(const ColourPalette& pal) : palette(pal)
    {
        setRange(1.0, 300.0, 1);
        setValue(30.0, juce::dontSendNotification);
    }

    std::function<void(double)> onValueChanged;
    std::function<void(double)> onDragEnd;

    void setValue(double newValue, juce::NotificationType notification)
    {
        const double constrainedValue = juce::jlimit(minValue, maxValue, newValue);
        if (currentValue != constrainedValue)
        {
            currentValue = constrainedValue;
            repaint();
            if (notification == juce::sendNotification && onValueChanged)
            {
                onValueChanged(currentValue);
            }
        }
    }

    void setRange(double min, double max, double step)
    {
        minValue = min;
        maxValue = max;
        stepValue = step;
    }

    double getValue() const { return currentValue; }

    void paint(juce::Graphics& g) override
    {
        // Transparent background
        g.fillAll(juce::Colours::transparentBlack);

        // Use the new text color and a suitable font
        g.setColour(palette.controlText);
        juce::Font customFont("Arial", 22.0f, juce::Font::bold);
        g.setFont(customFont);

        juce::String textToDraw = juce::String(currentValue, 1) + "s";
        g.drawFittedText(textToDraw, getLocalBounds(), juce::Justification::centredLeft, 1);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        dragStartY = event.getScreenY();
        valueAtDragStart = currentValue;
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        int dragDistanceY = dragStartY - event.getScreenY();
        double newValue = valueAtDragStart + (dragDistanceY * stepValue);
        setValue(newValue, juce::sendNotification);
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        if (event.mouseWasDraggedSinceMouseDown() && onDragEnd)
        {
            onDragEnd(currentValue);
        }
    }

private:
    const ColourPalette& palette;
    double currentValue = 0.0, minValue = 0.0, maxValue = 100.0, stepValue = 0.1;
    double valueAtDragStart = 0.0;
    int dragStartY = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DraggableNumberBox)
};