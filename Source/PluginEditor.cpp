#include "PluginEditor.h"

//==============================================================================
// VolumeBarGraph Implementation
//==============================================================================

VolumeBarGraph::VolumeBarGraph()
{
    values.fill(0.5f);
}

VolumeBarGraph::~VolumeBarGraph() {}

void VolumeBarGraph::setActiveCount(int count)
{
    activeCount = juce::jlimit(1, MAX_DELAY_COUNT, count);
    repaint();
}

float VolumeBarGraph::getBarValue(int index) const
{
    if (index < 0 || index >= MAX_DELAY_COUNT)
        return 0.0f;
    return values[index];
}

int VolumeBarGraph::getBarIndexAt(juce::Point<float> pos) const
{
    float barWidth = (float)getWidth() / MAX_DELAY_COUNT;
    int index = (int)(pos.x / barWidth);
    return juce::jlimit(0, MAX_DELAY_COUNT - 1, index);
}

void VolumeBarGraph::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 30));

    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawText(label, 4, 0, 60, 16, juce::Justification::centredLeft, false);

    g.setColour(juce::Colour(80, 80, 80));
    g.drawRect(getLocalBounds(), 1);

    float labelH = 16.0f;
    float barArea = getHeight() - labelH;
    float barWidth = (float)getWidth() / MAX_DELAY_COUNT;

    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        float x = i * barWidth;
        float barH = barArea * values[i];
        float y = labelH + (barArea - barH);
        bool  isActive = i < activeCount;

        g.setColour(isActive ? juce::Colour(40, 40, 50) : juce::Colour(35, 35, 35));
        g.fillRect(x + 1, labelH, barWidth - 2, barArea);

        g.setColour(isActive ? juce::Colour(50, 205, 50) : juce::Colour(30, 60, 30));
        g.fillRect(x + 1, y, barWidth - 2, barH);

        g.setColour(juce::Colour(20, 20, 20));
        g.drawLine(x, labelH, x, (float)getHeight(), 1.0f);
    }
}

void VolumeBarGraph::resized() {}

void VolumeBarGraph::setLabel(const juce::String& text)
{
    label = text;
    repaint();
}

void VolumeBarGraph::mouseDown(const juce::MouseEvent& e)
{
    int index = getBarIndexAt(e.position);
    if (index < activeCount)
    {
        draggedBar = index;
        float labelH = 16.0f;
        float newVal = 1.0f - ((e.position.y - labelH) / (getHeight() - labelH));
        values[draggedBar] = juce::jlimit(0.0f, 1.0f, newVal);
        repaint();
    }
}

void VolumeBarGraph::mouseDrag(const juce::MouseEvent& e)
{
    if (draggedBar == -1)
        return;

    int currentBar = getBarIndexAt(e.position);
    if (currentBar < activeCount)
    {
        float labelH = 16.0f;
        float newVal = 1.0f - ((e.position.y - labelH) / (getHeight() - labelH));
        values[currentBar] = juce::jlimit(0.0f, 1.0f, newVal);
    }
    repaint();
}

void VolumeBarGraph::mouseUp(const juce::MouseEvent&)
{
    draggedBar = -1;
}

juce::String VolumeBarGraph::serialize() const
{
    juce::String result;
    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        result += juce::String(values[i]);
        if (i < MAX_DELAY_COUNT - 1)
            result += ",";
    }
    return result;
}

void VolumeBarGraph::deserialize(const juce::String& data)
{
    if (data.isEmpty())
        return;
    auto tokens = juce::StringArray::fromTokens(data, ",", "");
    for (int i = 0; i < juce::jmin((int)tokens.size(), MAX_DELAY_COUNT); ++i)
        values[i] = juce::jlimit(0.0f, 1.0f, tokens[i].getFloatValue());
    repaint();
}

//==============================================================================
// PanBarGraph Implementation
//==============================================================================

PanBarGraph::PanBarGraph()
{
    values.fill(0.0f);
}

PanBarGraph::~PanBarGraph() {}

void PanBarGraph::setActiveCount(int count)
{
    activeCount = juce::jlimit(1, MAX_DELAY_COUNT, count);
    repaint();
}

float PanBarGraph::getBarValue(int index) const
{
    if (index < 0 || index >= MAX_DELAY_COUNT)
        return 0.0f;
    return values[index];
}

int PanBarGraph::getBarIndexAt(juce::Point<float> pos) const
{
    float barWidth = (float)getWidth() / MAX_DELAY_COUNT;
    int index = (int)(pos.x / barWidth);
    return juce::jlimit(0, MAX_DELAY_COUNT - 1, index);
}

void PanBarGraph::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 30));

    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawText(label, 4, 0, 60, 16, juce::Justification::centredLeft, false);

    g.setColour(juce::Colour(80, 80, 80));
    g.drawRect(getLocalBounds(), 1);

    float labelH = 16.0f;
    float barArea = getHeight() - labelH;
    float barWidth = (float)getWidth() / MAX_DELAY_COUNT;
    float centerY = labelH + barArea / 2.0f;

    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        float x = i * barWidth;
        bool isActive = i < activeCount;
        float val = values[i];

        g.setColour(isActive ? juce::Colour(40, 40, 50) : juce::Colour(35, 35, 35));
        g.fillRect(x + 1, labelH, barWidth - 2, barArea);

        g.setColour(juce::Colour(80, 80, 80));
        g.drawLine(x + 1, centerY, x + barWidth - 1, centerY, 1.0f);

        float fillH = std::abs(val) * (barArea / 2.0f);
        float fillY = val <= 0.0f ? centerY - fillH : centerY;

        g.setColour(isActive ? juce::Colour(50, 205, 50) : juce::Colour(30, 60, 30));
        g.fillRect(x + 1, fillY, barWidth - 2, fillH);

        g.setColour(juce::Colour(20, 20, 20));
        g.drawLine(x, labelH, x, (float)getHeight(), 1.0f);
    }
}

void PanBarGraph::resized() {}

void PanBarGraph::setLabel(const juce::String& text)
{
    label = text;
    repaint();
}

void PanBarGraph::mouseDown(const juce::MouseEvent& e)
{
    int index = getBarIndexAt(e.position);
    if (index < activeCount)
    {
        draggedBar = index;
        float labelH = 16.0f;
        float newVal = ((e.position.y - labelH) / (getHeight() - labelH)) * 2.0f - 1.0f;
        values[draggedBar] = juce::jlimit(-1.0f, 1.0f, newVal);
        repaint();
    }
}

void PanBarGraph::mouseDrag(const juce::MouseEvent& e)
{
    if (draggedBar == -1)
        return;

    int currentBar = getBarIndexAt(e.position);
    if (currentBar < activeCount)
    {
        float labelH = 16.0f;
        float newVal = ((e.position.y - labelH) / (getHeight() - labelH)) * 2.0f - 1.0f;
        values[currentBar] = juce::jlimit(-1.0f, 1.0f, newVal);
    }
    repaint();
}

void PanBarGraph::mouseUp(const juce::MouseEvent&)
{
    draggedBar = -1;
}

juce::String PanBarGraph::serialize() const
{
    juce::String result;
    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        result += juce::String(values[i]);
        if (i < MAX_DELAY_COUNT - 1)
            result += ",";
    }
    return result;
}

void PanBarGraph::deserialize(const juce::String& data)
{
    if (data.isEmpty())
        return;
    auto tokens = juce::StringArray::fromTokens(data, ",", "");
    for (int i = 0; i < juce::jmin((int)tokens.size(), MAX_DELAY_COUNT); ++i)
        values[i] = juce::jlimit(-1.0f, 1.0f, tokens[i].getFloatValue());
    repaint();
}

//==============================================================================
// Editor Implementation
//==============================================================================

CDelayAudioProcessorEditor::CDelayAudioProcessorEditor(CDelayAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    auto setupKnob = [this](juce::Slider& slider, juce::Label& label,
        const juce::String& labelText)
        {
            slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
            addAndMakeVisible(slider);

            label.setText(labelText, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(label);
        };

    setupKnob(delayCountSlider, delayCountLabel, "Count");
    setupKnob(delayTimeSlider, delayTimeLabel, "Time (ms)");
    setupKnob(inputVolumeSlider, inputVolumeLabel, "Input Vol");
    setupKnob(dryWetSlider, dryWetLabel, "Dry/Wet");
    setupKnob(outputVolumeSlider, outputVolumeLabel, "Output Vol");
    setupKnob(filterSlider, filterLabel, "Filter");
    setupKnob(sendSlider,  sendLabel,  "Send");
    setupKnob(swingSlider, swingLabel, "Swing");

    delayCountSlider.setRange(1, MAX_DELAY_COUNT, 1);

    delayCountAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "delayCount", delayCountSlider);
    delayTimeAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "delayTime", delayTimeSlider);
    inputVolumeAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "inputVolume", inputVolumeSlider);
    dryWetAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "dryWet", dryWetSlider);
    outputVolumeAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "outputVolume", outputVolumeSlider);
    filterAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterCutoff", filterSlider);
    sendAttachment  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "send",  sendSlider);
    swingAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "swing", swingSlider);

    bpmSyncButton.setButtonText("BPM Sync");
    addAndMakeVisible(bpmSyncButton);
    bpmSyncAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, "bpmSync", bpmSyncButton);

    filterDelaysOnlyButton.setButtonText("Delays Only");
    addAndMakeVisible(filterDelaysOnlyButton);
    filterDelaysOnlyAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, "filterDelaysOnly", filterDelaysOnlyButton);

    feedbackBarGraph.setLabel("Feedback");
    widthBarGraph.setLabel   ("Width");

    addAndMakeVisible(noteDivisionLabel);
    addAndMakeVisible(volumeBarGraph);
    addAndMakeVisible(panBarGraph);
    addAndMakeVisible(feedbackBarGraph);
    addAndMakeVisible(perTapFilterGraph);
    addAndMakeVisible(widthBarGraph);
    addAndMakeVisible(volumeTab);
    addAndMakeVisible(panTab);
    addAndMakeVisible(feedbackTab);
    addAndMakeVisible(tapFilterTab);
    addAndMakeVisible(widthTab);

    auto showOnly = [this](int tab)
    {
        activeTab = tab;
        volumeBarGraph.setVisible   (tab == 0);
        panBarGraph.setVisible      (tab == 1);
        feedbackBarGraph.setVisible (tab == 2);
        perTapFilterGraph.setVisible(tab == 3);
        widthBarGraph.setVisible    (tab == 4);
        updateTabAppearance();
    };

    volumeTab.onClick    = [showOnly]() { showOnly(0); };
    panTab.onClick       = [showOnly]() { showOnly(1); };
    feedbackTab.onClick  = [showOnly]() { showOnly(2); };
    tapFilterTab.onClick = [showOnly]() { showOnly(3); };
    widthTab.onClick     = [showOnly]() { showOnly(4); };

    showOnly(0);

    setSize(730, 500);
    startTimerHz(30);

    stateLoaded = false;

    bool initBpmSync = audioProcessor.apvts.getRawParameterValue("bpmSync")->load() > 0.5f;
    wasBpmSync = initBpmSync;

    if (initBpmSync)
    {
        delayTimeSlider.setRange(1, 10, 1);
        delayTimeSlider.setValue(audioProcessor.lastDivisionValue, juce::dontSendNotification);
        delayTimeLabel.setText("Division", juce::dontSendNotification);
    }
    else
    {
        delayTimeSlider.setRange(1, 2000, 1);
        delayTimeSlider.setValue(audioProcessor.lastMsValue, juce::dontSendNotification);
        delayTimeLabel.setText("Time (ms)", juce::dontSendNotification);
    }
}

CDelayAudioProcessorEditor::~CDelayAudioProcessorEditor() {}

void CDelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(40, 40, 40));
}

void CDelayAudioProcessorEditor::resized()
{
    int knobSize = 80;
    int labelHeight = 20;
    int topRow = 10;

    delayCountSlider.setBounds(10, topRow, knobSize, knobSize);
    delayCountLabel.setBounds(10, topRow + knobSize, knobSize, labelHeight);

    delayTimeSlider.setBounds(100, topRow, knobSize, knobSize);
    delayTimeLabel.setBounds(100, topRow + knobSize, knobSize, labelHeight);

    inputVolumeSlider.setBounds(190, topRow, knobSize, knobSize);
    inputVolumeLabel.setBounds(190, topRow + knobSize, knobSize, labelHeight);

    dryWetSlider.setBounds(280, topRow, knobSize, knobSize);
    dryWetLabel.setBounds(280, topRow + knobSize, knobSize, labelHeight);

    outputVolumeSlider.setBounds(370, topRow, knobSize, knobSize);
    outputVolumeLabel.setBounds(370, topRow + knobSize, knobSize, labelHeight);

    filterSlider.setBounds(460, topRow, knobSize, knobSize);
    filterLabel.setBounds(460, topRow + knobSize, knobSize, labelHeight);

    bpmSyncButton.setBounds(100, topRow + knobSize + labelHeight + 5, 80, 20);

    filterDelaysOnlyButton.setBounds(460, topRow + knobSize + labelHeight + 5, 90, 20);

    sendSlider.setBounds (550, topRow, knobSize, knobSize);
    sendLabel.setBounds  (550, topRow + knobSize, knobSize, labelHeight);

    swingSlider.setBounds(640, topRow, knobSize, knobSize);
    swingLabel.setBounds (640, topRow + knobSize, knobSize, labelHeight);

    int graphTop = topRow + knobSize + labelHeight + 70;

    int tabH = 24;
    volumeTab.setBounds     (10,  graphTop, 80, tabH);
    panTab.setBounds        (94,  graphTop, 80, tabH);
    feedbackTab.setBounds   (178, graphTop, 80, tabH);
    tapFilterTab.setBounds  (262, graphTop, 80, tabH);
    widthTab.setBounds      (346, graphTop, 80, tabH);

    int graphAreaTop = graphTop + tabH + 4;
    int graphHeight  = getHeight() - graphAreaTop - 10;

    volumeBarGraph.setBounds   (10, graphAreaTop, getWidth() - 20, graphHeight);
    panBarGraph.setBounds      (10, graphAreaTop, getWidth() - 20, graphHeight);
    feedbackBarGraph.setBounds (10, graphAreaTop, getWidth() - 20, graphHeight);
    perTapFilterGraph.setBounds(10, graphAreaTop, getWidth() - 20, graphHeight);
    widthBarGraph.setBounds    (10, graphAreaTop, getWidth() - 20, graphHeight);
}

void CDelayAudioProcessorEditor::timerCallback()
{
    if (!stateLoaded)
    {
        juce::String barData;
        for (int i = 0; i < MAX_DELAY_COUNT; ++i)
        {
            barData += juce::String(audioProcessor.barValues[i]);
            if (i < MAX_DELAY_COUNT - 1)
                barData += ",";
        }
        volumeBarGraph.deserialize(barData);

        juce::String panData;
        for (int i = 0; i < MAX_DELAY_COUNT; ++i)
        {
            panData += juce::String(audioProcessor.panValues[i]);
            if (i < MAX_DELAY_COUNT - 1)
                panData += ",";
        }
        panBarGraph.deserialize(panData);

        juce::String feedbackData;
        for (int i = 0; i < MAX_DELAY_COUNT; ++i)
        {
            feedbackData += juce::String(audioProcessor.feedbackValues[i]);
            if (i < MAX_DELAY_COUNT - 1)
                feedbackData += ",";
        }
        feedbackBarGraph.deserialize(feedbackData);

        juce::String tapFilterData;
        for (int i = 0; i < MAX_DELAY_COUNT; ++i)
        {
            float graphVal = 1.0f - audioProcessor.perTapFilterValues[i] * 2.0f;
            tapFilterData += juce::String(graphVal);
            if (i < MAX_DELAY_COUNT - 1)
                tapFilterData += ",";
        }
        perTapFilterGraph.deserialize(tapFilterData);

        juce::String widthData;
        for (int i = 0; i < MAX_DELAY_COUNT; ++i)
        {
            widthData += juce::String(audioProcessor.widthValues[i]); // already -1 to 1
            if (i < MAX_DELAY_COUNT - 1)
                widthData += ",";
        }
        widthBarGraph.deserialize(widthData);

        bool initBpmSync = audioProcessor.apvts.getRawParameterValue("bpmSync")->load() > 0.5f;
        wasBpmSync = initBpmSync;
        if (initBpmSync)
        {
            delayTimeSlider.setRange(1, 10, 1);
            delayTimeSlider.setValue(audioProcessor.lastDivisionValue, juce::dontSendNotification);
            delayTimeLabel.setText("Division", juce::dontSendNotification);
        }
        else
        {
            delayTimeSlider.setRange(1, 2000, 1);
            delayTimeSlider.setValue(audioProcessor.lastMsValue, juce::dontSendNotification);
            delayTimeLabel.setText("Time (ms)", juce::dontSendNotification);
        }

        stateLoaded = true;
    }

    int count = (int)audioProcessor.apvts.getRawParameterValue("delayCount")->load();
    volumeBarGraph.setActiveCount(count);
    panBarGraph.setActiveCount(count);
    feedbackBarGraph.setActiveCount(count);
    perTapFilterGraph.setActiveCount(count);
    widthBarGraph.setActiveCount(count);

    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        audioProcessor.barValues[i]          = volumeBarGraph.getBarValue(i);
        audioProcessor.panValues[i]          = panBarGraph.getBarValue(i);
        audioProcessor.feedbackValues[i]     = feedbackBarGraph.getBarValue(i);
        audioProcessor.perTapFilterValues[i] = (1.0f - perTapFilterGraph.getBarValue(i)) * 0.5f;
        audioProcessor.widthValues[i]         = widthBarGraph.getBarValue(i);
    }

    for (int i = 0; i < count; ++i)
        audioProcessor.repeatGains[i] = audioProcessor.barValues[i];

    bool bpmSync = audioProcessor.apvts.getRawParameterValue("bpmSync")->load() > 0.5f;

    if (bpmSync != wasBpmSync)
    {
        if (bpmSync)
        {
            audioProcessor.lastMsValue = (float)delayTimeSlider.getValue();
            delayTimeSlider.setRange(1, 10, 1);
            delayTimeSlider.setValue(audioProcessor.lastDivisionValue, juce::dontSendNotification);
            delayTimeLabel.setText("Division", juce::dontSendNotification);
        }
        else
        {
            audioProcessor.lastDivisionValue = (float)delayTimeSlider.getValue();
            delayTimeSlider.setRange(1, 2000, 1);
            delayTimeSlider.setValue(audioProcessor.lastMsValue, juce::dontSendNotification);
            delayTimeLabel.setText("Time (ms)", juce::dontSendNotification);
        }
        wasBpmSync = bpmSync;
    }

    if (bpmSync)
    {
        audioProcessor.lastDivisionValue = (float)delayTimeSlider.getValue();

        int knobPos = (int)delayTimeSlider.getValue() - 1;
        float targetVal = 0.0f;
        if (auto* param = audioProcessor.apvts.getParameter("noteDivision"))
        {
            targetVal = param->convertTo0to1((float)knobPos);
            if (std::abs(param->getValue() - targetVal) > 0.001f)
                param->setValueNotifyingHost(targetVal);
        }
    }
    else
    {
        audioProcessor.lastMsValue = (float)delayTimeSlider.getValue();
    }
}

void CDelayAudioProcessorEditor::updateTabAppearance()
{
    auto activeColour   = juce::Colour(70, 130, 180);
    auto inactiveColour = juce::Colour(50, 50, 55);

    volumeTab.setColour   (juce::TextButton::buttonColourId, activeTab == 0 ? activeColour : inactiveColour);
    panTab.setColour      (juce::TextButton::buttonColourId, activeTab == 1 ? activeColour : inactiveColour);
    feedbackTab.setColour (juce::TextButton::buttonColourId, activeTab == 2 ? activeColour : inactiveColour);
    tapFilterTab.setColour(juce::TextButton::buttonColourId, activeTab == 3 ? activeColour : inactiveColour);
    widthTab.setColour    (juce::TextButton::buttonColourId, activeTab == 4 ? activeColour : inactiveColour);
}