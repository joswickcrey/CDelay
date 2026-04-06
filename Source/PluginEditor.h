#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

struct Node
{
    float x;
    float y;
    float tension;
};

class VolumeBarGraph : public juce::Component
{
public:
    VolumeBarGraph();
    ~VolumeBarGraph() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void setActiveCount(int count);
    float getBarValue(int index) const;
    void setLabel(const juce::String& text);
    void resetAll(float value);

    juce::String serialize() const;
    void deserialize(const juce::String& data);

    std::function<void()> onBarChanged;

private:
    std::array<float, MAX_DELAY_COUNT> values;
    int activeCount = 4;
    int draggedBar = -1;
    juce::String label = "Volume";

    int getBarIndexAt(juce::Point<float> pos) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VolumeBarGraph)
};

class PanBarGraph : public juce::Component
{
public:
    PanBarGraph();
    ~PanBarGraph() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void setActiveCount(int count);
    float getBarValue(int index) const;
    void setLabel(const juce::String& text);
    void resetAll(float value);

    juce::String serialize() const;
    void deserialize(const juce::String& data);

private:
    std::array<float, MAX_DELAY_COUNT> values;
    int activeCount = 4;
    int draggedBar = -1;
    juce::String label = "Pan";

    int getBarIndexAt(juce::Point<float> pos) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PanBarGraph)
};

class CDelayAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   public juce::Timer
{
public:
    CDelayAudioProcessorEditor(CDelayAudioProcessor&);
    ~CDelayAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    CDelayAudioProcessor& audioProcessor;

    juce::Slider delayCountSlider;
    juce::Slider delayTimeSlider;
    juce::Slider inputVolumeSlider;
    juce::Slider dryWetSlider;
    juce::Slider outputVolumeSlider;
    juce::Slider filterSlider;

    juce::Label delayCountLabel;
    juce::Label delayTimeLabel;
    juce::Label inputVolumeLabel;
    juce::Label dryWetLabel;
    juce::Label outputVolumeLabel;
    juce::Label filterLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> delayCountAttachment;
    std::unique_ptr<SliderAttachment> delayTimeAttachment;
    std::unique_ptr<SliderAttachment> inputVolumeAttachment;
    std::unique_ptr<SliderAttachment> dryWetAttachment;
    std::unique_ptr<SliderAttachment> outputVolumeAttachment;
    std::unique_ptr<SliderAttachment> filterAttachment;

    juce::ToggleButton bpmSyncButton;
    juce::ComboBox noteDivisionCombo;
    juce::Label noteDivisionLabel;

    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<ButtonAttachment> bpmSyncAttachment;
    std::unique_ptr<ComboAttachment>  noteDivisionAttachment;

    VolumeBarGraph volumeBarGraph;
    PanBarGraph    panBarGraph;
    VolumeBarGraph feedbackBarGraph;
    PanBarGraph    perTapFilterGraph;
    PanBarGraph    widthBarGraph;
    bool stateLoaded = false;

    juce::ToggleButton filterDelaysOnlyButton;
    std::unique_ptr<ButtonAttachment> filterDelaysOnlyAttachment;

    juce::Slider sendSlider;
    juce::Label  sendLabel;
    std::unique_ptr<SliderAttachment> sendAttachment;

    juce::Slider swingSlider;
    juce::Label  swingLabel;
    std::unique_ptr<SliderAttachment> swingAttachment;


    bool wasBpmSync = false;

    juce::TextButton volumeTab      { "Volume" };
    juce::TextButton panTab         { "Pan" };
    juce::TextButton feedbackTab    { "Feedback" };
    juce::TextButton tapFilterTab   { "Filter" };
    juce::TextButton widthTab       { "Width" };
    juce::TextButton resetButton   { "Reset" };

    std::array<std::unique_ptr<juce::Slider>, MAX_DELAY_COUNT> fbTimingKnobs;
    std::array<std::unique_ptr<juce::ToggleButton>, MAX_DELAY_COUNT> fbTimingSyncToggles;
    int activeTab = 0;
    void updateTabAppearance();
    juce::Rectangle<int> screenRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CDelayAudioProcessorEditor)
};