/**
 *
 *  @file      CustomLookAndFeel.cpp
 *  @author    Hallvard Jensen
 *  @date      8 Aug 2021 3:09:40pm
 *  @copyright © Hallvard Jensen, 2021. All right reserved.
 *
 */

#include "CustomLookAndFeel.h"

using namespace anyMidi;


CustomLookaAndFeel::CustomLookaAndFeel()
{

    setColour(juce::DocumentWindow::backgroundColourId, juce::Colours::red);
    // Tabs
    setColour(juce::TabbedComponent::ColourIds::outlineColourId, palette.at(ColorGroup::BackgroundPrimary));
    setColour(juce::TabbedComponent::ColourIds::backgroundColourId, palette.at(ColorGroup::BackgroundPrimary));

    setColour(juce::TabbedButtonBar::ColourIds::tabOutlineColourId, palette.at(ColorGroup::Outline));
    setColour(juce::TabbedButtonBar::ColourIds::tabTextColourId, palette.at(ColorGroup::TextSecondary));
    setColour(juce::TabbedButtonBar::ColourIds::frontTextColourId, palette.at(ColorGroup::TextPrimary));

    // Label
    setColour(juce::Label::ColourIds::textColourId, palette.at(ColorGroup::TextPrimary));

    // Slider
    setColour(juce::Slider::ColourIds::textBoxTextColourId, palette.at(ColorGroup::Active)); // Text sliders / boxes
    setColour(juce::Slider::ColourIds::trackColourId, palette.at(ColorGroup::Active));
    setColour(juce::Slider::ColourIds::backgroundColourId, palette.at(ColorGroup::BackgroundSecondary));
    setColour(juce::Slider::ColourIds::textBoxOutlineColourId, palette.at(ColorGroup::Outline));
    setColour(juce::Slider::ColourIds::thumbColourId, palette.at(ColorGroup::Active));

    // Button
    setColour(juce::TextButton::ColourIds::textColourOffId, palette.at(ColorGroup::TextPrimary));
    setColour(juce::TextButton::ColourIds::buttonColourId, palette.at(ColorGroup::BackgroundPrimary));
    setColour(juce::TextButton::ColourIds::buttonOnColourId, palette.at(ColorGroup::BackgroundSecondary));

    // Dropdown menu
    setColour(juce::ComboBox::ColourIds::textColourId, palette.at(ColorGroup::TextPrimary));
    setColour(juce::ComboBox::ColourIds::backgroundColourId, palette.at(ColorGroup::BackgroundPrimary));
    setColour(juce::ComboBox::ColourIds::arrowColourId, palette.at(ColorGroup::TextPrimary));
    setColour(juce::ComboBox::ColourIds::outlineColourId, palette.at(ColorGroup::Outline));

    setColour(juce::PopupMenu::ColourIds::backgroundColourId, palette.at(ColorGroup::BackgroundPrimary));
    setColour(juce::PopupMenu::ColourIds::textColourId, palette.at(ColorGroup::TextPrimary));
    setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, palette.at(ColorGroup::Active));
    setColour(juce::PopupMenu::ColourIds::highlightedTextColourId, palette.at(ColorGroup::BackgroundSecondary));

    // Checkboxes
    setColour(juce::ListBox::ColourIds::backgroundColourId, palette.at(ColorGroup::BackgroundPrimary));
    setColour(juce::ListBox::ColourIds::textColourId, palette.at(ColorGroup::TextPrimary));
    setColour(juce::ListBox::ColourIds::outlineColourId, palette.at(ColorGroup::Outline));

    setColour(juce::ToggleButton::ColourIds::textColourId, palette.at(ColorGroup::TextPrimary));
    setColour(juce::ToggleButton::ColourIds::tickColourId, palette.at(ColorGroup::TextPrimary));
    setColour(juce::ToggleButton::ColourIds::tickDisabledColourId, palette.at(ColorGroup::Outline));

    // Text box
    setColour(juce::TextEditor::ColourIds::backgroundColourId, palette.at(ColorGroup::BackgroundPrimary));
    setColour(juce::TextEditor::ColourIds::outlineColourId, palette.at(ColorGroup::Outline));
    setColour(juce::TextEditor::ColourIds::highlightColourId, palette.at(ColorGroup::Active));
    setColour(juce::TextEditor::ColourIds::highlightedTextColourId, palette.at(ColorGroup::BackgroundPrimary));
    setColour(juce::TextEditor::ColourIds::textColourId, palette.at(ColorGroup::TextPrimary));

    setColour(juce::CaretComponent::ColourIds::caretColourId, palette.at(ColorGroup::Active));
}