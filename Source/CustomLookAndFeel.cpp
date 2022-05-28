/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Created: 8 Aug 2021 3:09:40pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#include "CustomLookAndFeel.h"

using namespace anyMidi;

CustomLookaAndFeel::CustomLookaAndFeel()
{    
    // Tabs
    setColour(juce::TabbedComponent::ColourIds::outlineColourId, white);
    setColour(juce::TabbedComponent::ColourIds::backgroundColourId, white);

    setColour(juce::TabbedButtonBar::ColourIds::tabOutlineColourId, gray);
    setColour(juce::TabbedButtonBar::ColourIds::tabTextColourId, lightGray);
    setColour(juce::TabbedButtonBar::ColourIds::frontTextColourId, black);

    // Label
    setColour(juce::Label::textColourId, black);

    // Slider
    setColour(juce::Slider::ColourIds::textBoxTextColourId, black); // Text sliders / boxes
    setColour(juce::Slider::ColourIds::trackColourId, gray);
    setColour(juce::Slider::ColourIds::backgroundColourId, lightGray);
    setColour(juce::Slider::ColourIds::textBoxOutlineColourId, gray);
    setColour(juce::Slider::ColourIds::thumbColourId, gray);

    // Button
    setColour(juce::TextButton::ColourIds::textColourOffId, black);
    setColour(juce::TextButton::ColourIds::buttonColourId, white);
    setColour(juce::TextButton::ColourIds::buttonOnColourId, lightGray);

    // Dropdown menu
    setColour(juce::ComboBox::ColourIds::textColourId, black);
    setColour(juce::ComboBox::ColourIds::backgroundColourId, white);
    setColour(juce::ComboBox::ColourIds::arrowColourId, black);
    setColour(juce::ComboBox::ColourIds::outlineColourId, gray);

    setColour(juce::PopupMenu::ColourIds::backgroundColourId, white);
    setColour(juce::PopupMenu::ColourIds::textColourId, black);
    setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, blue);
    setColour(juce::PopupMenu::ColourIds::highlightedTextColourId, white);

    // Checkboxes
    setColour(juce::ListBox::ColourIds::backgroundColourId, white);
    setColour(juce::ListBox::ColourIds::textColourId, black);
    setColour(juce::ListBox::ColourIds::outlineColourId, gray);

    setColour(juce::ToggleButton::ColourIds::textColourId, black);
    setColour(juce::ToggleButton::ColourIds::tickColourId, black);
    setColour(juce::ToggleButton::ColourIds::tickDisabledColourId, gray);

    // Text box
    setColour(juce::TextEditor::ColourIds::textColourId, black);
    setColour(juce::TextEditor::ColourIds::backgroundColourId, white);
    setColour(juce::TextEditor::ColourIds::outlineColourId, gray);
    setColour(juce::TextEditor::ColourIds::highlightColourId, black);
    setColour(juce::TextEditor::ColourIds::highlightedTextColourId, white);
}