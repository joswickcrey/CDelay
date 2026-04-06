#include "PluginEditor.h"

//==============================================================================
// VolumeBarGraph Implementation
// hello hi
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
    g.fillAll(juce::Colour(0, 0, 0));

    g.setColour(juce::Colour(0, 60, 0));
    g.drawRect(getLocalBounds(), 1);

    float barArea = (float)getHeight();
    float barWidth = (float)getWidth() / MAX_DELAY_COUNT;

    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        float x = i * barWidth;
        float barH = barArea * values[i];
        float y = barArea - barH;
        bool  isActive = i < activeCount;

        g.setColour(isActive ? juce::Colour(0, 15, 0) : juce::Colour(0, 5, 0));
        g.fillRect(x + 1, 0.0f, barWidth - 2, barArea);

        g.setColour(isActive ? juce::Colour(50, 205, 50) : juce::Colour(20, 50, 20));
        g.fillRect(x + 1, y, barWidth - 2, barH);

        g.setColour(juce::Colour(0, 30, 0));
        g.drawLine(x, 0.0f, x, (float)getHeight(), 1.0f);
    }
}

void VolumeBarGraph::resized() {}

void VolumeBarGraph::setLabel(const juce::String& text)
{
    label = text;
    repaint();
}

void VolumeBarGraph::resetAll(float value)
{
    values.fill(value);
    repaint();
}

void VolumeBarGraph::mouseDown(const juce::MouseEvent& e)
{
    int index = getBarIndexAt(e.position);
    if (index < activeCount)
    {
        draggedBar = index;
        float newVal = 1.0f - (e.position.y / (float)getHeight());
        values[draggedBar] = juce::jlimit(0.0f, 1.0f, newVal);
        if (onBarChanged) onBarChanged();
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
        float newVal = 1.0f - (e.position.y / (float)getHeight());
        values[currentBar] = juce::jlimit(0.0f, 1.0f, newVal);
        if (onBarChanged) onBarChanged();
    }
    repaint();
}

void VolumeBarGraph::mouseUp(const juce::MouseEvent&)
{
    draggedBar = -1;
}

void VolumeBarGraph::mouseDoubleClick(const juce::MouseEvent& e)
{
    int index = getBarIndexAt(e.position);
    if (index < activeCount)
    {
        values[index] = defaultVal;
        repaint();
    }
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
    g.fillAll(juce::Colour(0, 0, 0));

    g.setColour(juce::Colour(0, 60, 0));
    g.drawRect(getLocalBounds(), 1);

    float barArea = (float)getHeight();
    float barWidth = (float)getWidth() / MAX_DELAY_COUNT;
    float centerY = barArea / 2.0f;

    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        float x = i * barWidth;
        bool isActive = i < activeCount;
        float val = values[i];

        g.setColour(isActive ? juce::Colour(0, 15, 0) : juce::Colour(0, 5, 0));
        g.fillRect(x + 1, 0.0f, barWidth - 2, barArea);

        g.setColour(juce::Colour(0, 40, 0));
        g.drawLine(x + 1, centerY, x + barWidth - 1, centerY, 1.0f);

        float fillH = std::abs(val) * (barArea / 2.0f);
        float fillY = val <= 0.0f ? centerY - fillH : centerY;

        g.setColour(isActive ? juce::Colour(50, 205, 50) : juce::Colour(20, 50, 20));
        g.fillRect(x + 1, fillY, barWidth - 2, fillH);

        g.setColour(juce::Colour(0, 30, 0));
        g.drawLine(x, 0.0f, x, (float)getHeight(), 1.0f);
    }
}

void PanBarGraph::resized() {}

void PanBarGraph::setLabel(const juce::String& text)
{
    label = text;
    repaint();
}

void PanBarGraph::resetAll(float value)
{
    values.fill(value);
    repaint();
}

void PanBarGraph::mouseDown(const juce::MouseEvent& e)
{
    int index = getBarIndexAt(e.position);
    if (index < activeCount)
    {
        draggedBar = index;
        float newVal = (e.position.y / (float)getHeight()) * 2.0f - 1.0f;
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
        float newVal = (e.position.y / (float)getHeight()) * 2.0f - 1.0f;
        values[currentBar] = juce::jlimit(-1.0f, 1.0f, newVal);
    }
    repaint();
}

void PanBarGraph::mouseUp(const juce::MouseEvent&)
{
    draggedBar = -1;
}

void PanBarGraph::mouseDoubleClick(const juce::MouseEvent& e)
{
    int index = getBarIndexAt(e.position);
    if (index < activeCount)
    {
        values[index] = defaultVal;
        repaint();
    }
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
    feedbackBarGraph.setDefaultValue(0.0f);
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
    addAndMakeVisible(resetButton);

    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        fbTimingKnobs[i] = std::make_unique<juce::Slider>();
        fbTimingKnobs[i]->setSliderStyle(juce::Slider::RotaryVerticalDrag);
        fbTimingKnobs[i]->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        fbTimingKnobs[i]->setRange(0, 9, 1);
        fbTimingKnobs[i]->setValue(3.0, juce::dontSendNotification);
        fbTimingKnobs[i]->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(50, 205, 50));
        fbTimingKnobs[i]->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0, 40, 0));
        fbTimingKnobs[i]->setColour(juce::Slider::thumbColourId, juce::Colour(50, 205, 50));
        fbTimingKnobs[i]->setVisible(false);
        addAndMakeVisible(fbTimingKnobs[i].get());

        fbTimingSyncToggles[i] = std::make_unique<juce::ToggleButton>();
        fbTimingSyncToggles[i]->setToggleState(true, juce::dontSendNotification);
        fbTimingSyncToggles[i]->setColour(juce::ToggleButton::tickColourId, juce::Colour(50, 205, 50));
        fbTimingSyncToggles[i]->setVisible(false);
        addAndMakeVisible(fbTimingSyncToggles[i].get());
    }

    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0, 30, 0));
    resetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(50, 205, 50));

    resetButton.onClick = [this]()
    {
        switch (activeTab)
        {
            case 0: volumeBarGraph.resetAll(0.5f);   break;
            case 1: panBarGraph.resetAll(0.0f);       break;
            case 2:
                feedbackBarGraph.resetAll(0.0f);
                for (int i = 0; i < MAX_DELAY_COUNT; ++i)
                {
                    fbTimingKnobs[i]->setValue(3.0, juce::dontSendNotification);
                    fbTimingSyncToggles[i]->setToggleState(true, juce::dontSendNotification);
                }
                break;
            case 3: perTapFilterGraph.resetAll(0.0f); break;
            case 4: widthBarGraph.resetAll(0.0f);     break;
        }
    };

    auto showOnly = [this](int tab)
    {
        activeTab = tab;
        volumeBarGraph.setVisible   (tab == 0);
        panBarGraph.setVisible      (tab == 1);
        feedbackBarGraph.setVisible (tab == 2);
        perTapFilterGraph.setVisible(tab == 3);
        widthBarGraph.setVisible    (tab == 4);

        for (int i = 0; i < MAX_DELAY_COUNT; ++i)
        {
            fbTimingKnobs[i]->setVisible(tab == 2);
            fbTimingSyncToggles[i]->setVisible(tab == 2);
        }

        resized();
        updateTabAppearance();
    };

    volumeTab.onClick    = [showOnly]() { showOnly(0); };
    panTab.onClick       = [showOnly]() { showOnly(1); };
    feedbackTab.onClick  = [showOnly]() { showOnly(2); };
    tapFilterTab.onClick = [showOnly]() { showOnly(3); };
    widthTab.onClick     = [showOnly]() { showOnly(4); };

    showOnly(0);

    setSize(850, 450);
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

    g.setColour(juce::Colour(0, 0, 0));
    g.fillRoundedRectangle(screenRect.toFloat(), 4.0f);

    g.setColour(juce::Colour(0, 60, 0));
    g.drawRoundedRectangle(screenRect.toFloat(), 4.0f, 1.0f);
}

void CDelayAudioProcessorEditor::resized()
{
    int k = 80;   // knob size
    int lh = 16;  // label height
    int ky = 10;  // knob row y

    // 8 knobs evenly spaced across the window
    int gap = (getWidth() - 50 - 8 * k) / 7;
    auto knobX = [&](int i) { return 25 + i * (k + gap); };

    // Count | Division(+BPM) | DryWet | Swing || InputVol | Send | Filter(+DelaysOnly) | OutputVol
    delayCountSlider.setBounds   (knobX(0), ky, k, k);
    delayCountLabel.setBounds    (knobX(0), ky+k, k, lh);

    delayTimeSlider.setBounds    (knobX(1), ky, k, k);
    delayTimeLabel.setBounds     (knobX(1), ky+k, k, lh);
    bpmSyncButton.setBounds      (knobX(1), ky+k+lh+2, k, 20);

    dryWetSlider.setBounds       (knobX(2), ky, k, k);
    dryWetLabel.setBounds        (knobX(2), ky+k, k, lh);

    swingSlider.setBounds        (knobX(3), ky, k, k);
    swingLabel.setBounds         (knobX(3), ky+k, k, lh);

    inputVolumeSlider.setBounds  (knobX(4), ky, k, k);
    inputVolumeLabel.setBounds   (knobX(4), ky+k, k, lh);

    sendSlider.setBounds         (knobX(5), ky, k, k);
    sendLabel.setBounds          (knobX(5), ky+k, k, lh);

    filterSlider.setBounds       (knobX(6), ky, k, k);
    filterLabel.setBounds        (knobX(6), ky+k, k, lh);
    filterDelaysOnlyButton.setBounds(knobX(6), ky+k+lh+2, k, 20);

    outputVolumeSlider.setBounds (knobX(7), ky, k, k);
    outputVolumeLabel.setBounds  (knobX(7), ky+k, k, lh);

    // --- Tab bar (centered) ---
    int tabH = 24;
    int tabW = 80;
    int tabGap = 4;
    int numTabs = 5;
    int totalTabW = numTabs * tabW + (numTabs - 1) * tabGap;
    int tabX = (getWidth() - totalTabW) / 2;
    int tabY = ky + k + lh + 30;

    volumeTab.setBounds   (tabX,                          tabY, tabW, tabH);
    panTab.setBounds      (tabX + 1 * (tabW + tabGap),   tabY, tabW, tabH);
    feedbackTab.setBounds (tabX + 2 * (tabW + tabGap),   tabY, tabW, tabH);
    tapFilterTab.setBounds(tabX + 3 * (tabW + tabGap),   tabY, tabW, tabH);
    widthTab.setBounds    (tabX + 4 * (tabW + tabGap),   tabY, tabW, tabH);

    // --- Screen container ---
    int margin = 20;
    int containerTop = tabY + tabH + 6;
    screenRect = juce::Rectangle<int>(margin, containerTop,
        getWidth() - 2 * margin, getHeight() - containerTop - margin);

    int pad = 15;
    int gx = screenRect.getX() + pad;
    int gy = screenRect.getY() + pad;
    int gw = screenRect.getWidth() - 2 * pad;
    int fbH = 68;
    int gh = screenRect.getHeight() - 2 * pad - fbH;

    resetButton.setBounds(tabX + totalTabW + 10, tabY, 55, tabH);

    volumeBarGraph.setBounds   (gx, gy, gw, gh);
    panBarGraph.setBounds      (gx, gy, gw, gh);
    feedbackBarGraph.setBounds (gx, gy, gw, gh);
    perTapFilterGraph.setBounds(gx, gy, gw, gh);
    widthBarGraph.setBounds    (gx, gy, gw, gh);

    if (activeTab == 2)
    {
        float colW = (float)gw / MAX_DELAY_COUNT;
        int knobTop   = gy + gh + 2;
        int toggleTop = knobTop + 44;

        for (int i = 0; i < MAX_DELAY_COUNT; ++i)
        {
            int cx = gx + (int)(i * colW);
            int cw = (int)colW;
            fbTimingKnobs[i]->setBounds      (cx + 1, knobTop,   cw - 2, 42);
            int toggleW = 20;
            int toggleX = cx + (cw - toggleW) / 2;
            fbTimingSyncToggles[i]->setBounds (toggleX, toggleTop, toggleW, 20);
        }
    }
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

        for (int i = 0; i < MAX_DELAY_COUNT; ++i)
        {
            fbTimingKnobs[i]->setValue(audioProcessor.fbTimingValues[i], juce::dontSendNotification);
            fbTimingSyncToggles[i]->setToggleState(audioProcessor.fbTimingSyncFlags[i] > 0.5f, juce::dontSendNotification);
        }

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

    int parentDiv = (int)audioProcessor.apvts.getRawParameterValue("noteDivision")->load();

    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        audioProcessor.barValues[i]          = volumeBarGraph.getBarValue(i);
        audioProcessor.panValues[i]          = panBarGraph.getBarValue(i);
        audioProcessor.feedbackValues[i]     = feedbackBarGraph.getBarValue(i);
        audioProcessor.perTapFilterValues[i] = (1.0f - perTapFilterGraph.getBarValue(i)) * 0.5f;
        audioProcessor.widthValues[i]        = widthBarGraph.getBarValue(i);

        bool synced = fbTimingSyncToggles[i]->getToggleState();
        audioProcessor.fbTimingSyncFlags[i] = synced ? 1.0f : 0.0f;

        if (synced)
        {
            fbTimingKnobs[i]->setValue((double)parentDiv, juce::dontSendNotification);
            fbTimingKnobs[i]->setEnabled(false);
        }
        else
        {
            fbTimingKnobs[i]->setEnabled(true);
        }

        audioProcessor.fbTimingValues[i] = (float)fbTimingKnobs[i]->getValue();
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
    auto activeColour   = juce::Colour(50, 205, 50);
    auto inactiveColour = juce::Colour(0, 40, 0);

    auto textColour = juce::Colour(0, 0, 0);
    auto inactiveTextColour = juce::Colour(50, 205, 50);

    auto setTab = [&](juce::TextButton& tab, bool active)
    {
        tab.setColour(juce::TextButton::buttonColourId,  active ? activeColour : inactiveColour);
        tab.setColour(juce::TextButton::textColourOffId, active ? textColour : inactiveTextColour);
    };

    setTab(volumeTab,    activeTab == 0);
    setTab(panTab,       activeTab == 1);
    setTab(feedbackTab,  activeTab == 2);
    setTab(tapFilterTab, activeTab == 3);
    setTab(widthTab,     activeTab == 4);
}