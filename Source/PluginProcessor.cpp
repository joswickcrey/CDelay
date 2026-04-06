#include "PluginProcessor.h"
#include "PluginEditor.h"

CDelayAudioProcessor::CDelayAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    repeatGains.fill(1.0f);
    barValues.fill(0.5f);
    panValues.fill(0.0f);
    feedbackValues.fill(0.0f);
}

CDelayAudioProcessor::~CDelayAudioProcessor() {}

const juce::String CDelayAudioProcessor::getName() const { return JucePlugin_Name; }
bool CDelayAudioProcessor::acceptsMidi() const { return false; }
bool CDelayAudioProcessor::producesMidi() const { return false; }
bool CDelayAudioProcessor::isMidiEffect() const { return false; }
double CDelayAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int CDelayAudioProcessor::getNumPrograms() { return 1; }
int CDelayAudioProcessor::getCurrentProgram() { return 0; }
void CDelayAudioProcessor::setCurrentProgram(int) {}
const juce::String CDelayAudioProcessor::getProgramName(int) { return {}; }
void CDelayAudioProcessor::changeProgramName(int, const juce::String&) {}

void CDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    currentSampleRate = sampleRate;

    int maxDelaySamples = (int)((2000.0f / 1000.0f) * sampleRate);
    int totalBufferSize = 9 * maxDelaySamples;

    delayBuffer.setSize(getTotalNumOutputChannels(), totalBufferSize);
    delayBuffer.clear();

    writePosition = 0;

    inputVolumeSmoothed.reset(sampleRate, 0.05);
    dryWetSmoothed.reset(sampleRate, 0.05);
    outputVolumeSmoothed.reset(sampleRate, 0.05);

    inputVolumeSmoothed.setCurrentAndTargetValue(1.0f);
    dryWetSmoothed.setCurrentAndTargetValue(0.5f);
    outputVolumeSmoothed.setCurrentAndTargetValue(1.0f);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels = (juce::uint32)getTotalNumOutputChannels();

    lowPassFilterL.prepare(spec);
    lowPassFilterR.prepare(spec);
    highPassFilterL.prepare(spec);
    highPassFilterR.prepare(spec);

    *lowPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 20000.0f);
    *lowPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 20000.0f);

    *highPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
    *highPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);

    lowPassFilterL.reset();
    lowPassFilterR.reset();
    highPassFilterL.reset();
    highPassFilterR.reset();

    dryLowPassFilterL.prepare(spec);
    dryLowPassFilterR.prepare(spec);
    dryHighPassFilterL.prepare(spec);
    dryHighPassFilterR.prepare(spec);

    *dryLowPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 20000.0f);
    *dryLowPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 20000.0f);
    *dryHighPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
    *dryHighPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);

    dryLowPassFilterL.reset();
    dryLowPassFilterR.reset();
    dryHighPassFilterL.reset();
    dryHighPassFilterR.reset();

    filterSmoothed.reset(sampleRate, 0.05);
    filterSmoothed.setCurrentAndTargetValue(0.5f);

    wetBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    wetBuffer.clear();

    delayTimeSmoothed.reset(sampleRate, 0.05);
    delayTimeSmoothed.setCurrentAndTargetValue((float)delayTimeSamples);
}

void CDelayAudioProcessor::releaseResources()
{
    delayBuffer.setSize(0, 0);
}

void CDelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    wetBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, true, true);
    wetBuffer.clear();

    if (delayBuffer.getNumSamples() == 0 || delayBuffer.getNumChannels() == 0)
        return;

    if (auto* ph = getPlayHead())
    {
        if (auto position = ph->getPosition())
        {
            bool isPlaying = position->getIsPlaying();
            if (wasPlaying && !isPlaying)
            {
                delayBuffer.clear();
                writePosition = 0;
            }
            wasPlaying = isPlaying;
        }
    }

    delayCount = (int)apvts.getRawParameterValue("delayCount")->load();
    float delayTimeMs = apvts.getRawParameterValue("delayTime")->load();
    float sendAmount = apvts.getRawParameterValue("send")->load();

    inputVolumeSmoothed.setTargetValue(apvts.getRawParameterValue("inputVolume")->load());
    dryWetSmoothed.setTargetValue(apvts.getRawParameterValue("dryWet")->load());
    outputVolumeSmoothed.setTargetValue(apvts.getRawParameterValue("outputVolume")->load());
    filterSmoothed.setTargetValue(apvts.getRawParameterValue("filterCutoff")->load());

    bool bpmSync = apvts.getRawParameterValue("bpmSync")->load() > 0.5f;

    if (bpmSync)
    {
        double bpm = 120.0;
        if (auto* ph = getPlayHead())
        {
            if (auto position = ph->getPosition())
                if (auto bpmVal = position->getBpm())
                    bpm = *bpmVal;
        }

        const double divisions[] = {
            0.25, 0.5, 0.75, 1.0, 1.5,
            2.0,  3.0, 4.0,  6.0, 8.0
        };

        int divIndex = (int)apvts.getRawParameterValue("noteDivision")->load();
        double secondsPerBeat = 60.0 / bpm;
        double delaySeconds = secondsPerBeat * divisions[divIndex];
        delayTimeSmoothed.setTargetValue((float)(delaySeconds * currentSampleRate));
    }
    else
    {
        delayTimeSmoothed.setTargetValue((float)((delayTimeMs / 1000.0f) * currentSampleRate));
    }

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    const int bufferSize = delayBuffer.getNumSamples();

    float filterVal = 0.5f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float smoothedInput = inputVolumeSmoothed.getNextValue();
        float smoothedDryWet = dryWetSmoothed.getNextValue();
        float smoothedOutput = outputVolumeSmoothed.getNextValue();
        filterVal = filterSmoothed.getNextValue();

        float dryGain = juce::jlimit(0.0f, 1.0f, 2.0f * (1.0f - smoothedDryWet));
        int smoothedDelaySamples = (int)delayTimeSmoothed.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
        {
            float drySample = buffer.getSample(channel, sample) * smoothedInput;

            float feedbackSample = 0.0f;
            for (int repeat = 1; repeat <= delayCount; ++repeat)
            {
                int fbReadPos = writePosition - (repeat * smoothedDelaySamples);
                if (bufferSize > 0)
                    fbReadPos = ((fbReadPos % bufferSize) + bufferSize) % bufferSize;
                feedbackSample += delayBuffer.getSample(channel, fbReadPos)
                    * feedbackValues[repeat - 1];
            }

            delayBuffer.setSample(channel, writePosition, drySample * sendAmount + feedbackSample);

            float wetSample = 0.0f;
            for (int repeat = 1; repeat <= delayCount; ++repeat)
            {
                int readPosition = writePosition - (repeat * smoothedDelaySamples);

                if (bufferSize > 0)
                    readPosition = ((readPosition % bufferSize) + bufferSize) % bufferSize;
                else
                    continue;

                if (readPosition < 0 || readPosition >= bufferSize)
                    continue;

                float delayedSample = delayBuffer.getSample(channel, readPosition)
                    * repeatGains[repeat - 1];

                float pan = panValues[repeat - 1];
                float panAngle = (pan + 1.0f) * juce::MathConstants<float>::pi / 4.0f;
                float leftGain = std::cos(panAngle);
                float rightGain = std::sin(panAngle);
                float panGain = (channel == 0) ? leftGain : rightGain;

                wetSample += delayedSample * panGain;
            }

            buffer.setSample(channel, sample, drySample * dryGain * smoothedOutput);

            wetBuffer.setSample(channel, sample, wetSample);
        }

        writePosition = (writePosition + 1) % bufferSize;
    }

    bool filterDelaysOnly = apvts.getRawParameterValue("filterDelaysOnly")->load() > 0.5f;

    if (filterVal < 0.5f)
    {
        float t = 1.0f - (filterVal * 2.0f);
        float cutoff = juce::jmax(20.0f, 20000.0f * std::pow(200.0f / 20000.0f, t));
        *lowPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, cutoff);
        *lowPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, cutoff);

        auto* leftData = wetBuffer.getWritePointer(0);
        auto* rightData = wetBuffer.getWritePointer(1);
        for (int s = 0; s < numSamples; ++s)
        {
            leftData[s] = lowPassFilterL.processSample(leftData[s]);
            rightData[s] = lowPassFilterR.processSample(rightData[s]);
        }
    }
    else if (filterVal > 0.5f)
    {
        float t = (filterVal - 0.5f) * 2.0f;
        float cutoff = juce::jmin(20000.0f, 20.0f * std::pow(20000.0f / 20.0f, t));
        *highPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, cutoff);
        *highPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, cutoff);

        auto* leftData = wetBuffer.getWritePointer(0);
        auto* rightData = wetBuffer.getWritePointer(1);
        for (int s = 0; s < numSamples; ++s)
        {
            leftData[s] = highPassFilterL.processSample(leftData[s]);
            rightData[s] = highPassFilterR.processSample(rightData[s]);
        }
    }

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* outData = buffer.getWritePointer(channel);
        auto* wetData = wetBuffer.getReadPointer(channel);
        float wetGain = juce::jlimit(0.0f, 1.0f, 2.0f * dryWetSmoothed.getCurrentValue());
        float outVolume = outputVolumeSmoothed.getCurrentValue();

        for (int s = 0; s < numSamples; ++s)
            outData[s] += wetData[s] * wetGain * outVolume;
    }

    if (!filterDelaysOnly)
    {
        auto* leftData = buffer.getWritePointer(0);
        auto* rightData = buffer.getWritePointer(1);

        if (filterVal < 0.5f)
        {
            float t = 1.0f - (filterVal * 2.0f);
            float cutoff = juce::jmax(20.0f, 20000.0f * std::pow(200.0f / 20000.0f, t));
            *dryLowPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, cutoff);
            *dryLowPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, cutoff);

            for (int s = 0; s < numSamples; ++s)
            {
                leftData[s] = dryLowPassFilterL.processSample(leftData[s]);
                rightData[s] = dryLowPassFilterR.processSample(rightData[s]);
            }
        }
        else if (filterVal > 0.5f)
        {
            float t = (filterVal - 0.5f) * 2.0f;
            float cutoff = juce::jmin(20000.0f, 20.0f * std::pow(20000.0f / 20.0f, t));
            *dryHighPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, cutoff);
            *dryHighPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, cutoff);

            for (int s = 0; s < numSamples; ++s)
            {
                leftData[s] = dryHighPassFilterL.processSample(leftData[s]);
                rightData[s] = dryHighPassFilterR.processSample(rightData[s]);
            }
        }
    }
}

bool CDelayAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* CDelayAudioProcessor::createEditor()
{
    return new CDelayAudioProcessorEditor(*this);
}

void CDelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto graphXml = state.createXml();

    auto* barsXml = graphXml->createNewChildElement("VolumeBars");
    juce::String barData;
    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        barData += juce::String(barValues[i]);
        if (i < MAX_DELAY_COUNT - 1)
            barData += ",";
    }
    barsXml->setAttribute("data", barData);
    
    auto* panXml = graphXml->createNewChildElement("PanBars");
    juce::String panData;
    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        panData += juce::String(panValues[i]);
        if (i < MAX_DELAY_COUNT - 1)
            panData += ",";
    }
    panXml->setAttribute("data", panData);

    auto* feedbackXml = graphXml->createNewChildElement("FeedbackBars");
    juce::String feedbackData;
    for (int i = 0; i < MAX_DELAY_COUNT; ++i)
    {
        feedbackData += juce::String(feedbackValues[i]);
        if (i < MAX_DELAY_COUNT - 1)
            feedbackData += ",";
    }
    feedbackXml->setAttribute("data", feedbackData);

    auto* modeXml = graphXml->createNewChildElement("DelayTimeMode");
    modeXml->setAttribute("lastMs", lastMsValue);
    modeXml->setAttribute("lastDivision", lastDivisionValue);

    copyXmlToBinary(*graphXml, destData);
}

void CDelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));

        if (auto* barsXml = xmlState->getChildByName("VolumeBars"))
        {
            auto tokens = juce::StringArray::fromTokens(
                barsXml->getStringAttribute("data"), ",", "");
            for (int i = 0; i < juce::jmin((int)tokens.size(), MAX_DELAY_COUNT); ++i)
                barValues[i] = juce::jlimit(0.0f, 1.0f, tokens[i].getFloatValue());
        }

        if (auto* panXml = xmlState->getChildByName("PanBars"))
        {
            auto tokens = juce::StringArray::fromTokens(
                panXml->getStringAttribute("data"), ",", "");
            for (int i = 0; i < juce::jmin((int)tokens.size(), MAX_DELAY_COUNT); ++i)
                panValues[i] = juce::jlimit(-1.0f, 1.0f, tokens[i].getFloatValue());
        }

        if (auto* feedbackXml = xmlState->getChildByName("FeedbackBars"))
        {
            auto tokens = juce::StringArray::fromTokens(
                feedbackXml->getStringAttribute("data"), ",", "");
            for (int i = 0; i < juce::jmin((int)tokens.size(), MAX_DELAY_COUNT); ++i)
                feedbackValues[i] = juce::jlimit(0.0f, 1.0f, tokens[i].getFloatValue());
        }

        if (auto* modeXml = xmlState->getChildByName("DelayTimeMode"))
        {
            lastMsValue = (float)modeXml->getDoubleAttribute("lastMs", 500.0);
            lastDivisionValue = (float)modeXml->getDoubleAttribute("lastDivision", 4.0);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CDelayAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout CDelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "delayCount", "Delay Count", 1, MAX_DELAY_COUNT, 4));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delayTime", "Delay Time", 1.0f, 2000.0f, 500.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterCutoff", "Filter", 0.0f, 1.0f, 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "inputVolume", "Input Volume", 0.0f, 1.0f, 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "dryWet", "Dry/Wet", 0.0f, 1.0f, 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputVolume", "Output Volume", 0.0f, 1.0f, 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "bpmSync", "BPM Sync", true));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "noteDivision", "Note Division",
        juce::StringArray{
            "1/16", "1/8", "1/8d", "1/4", "1/4d",
            "1/2", "1/2d", "1/1", "1/1d", "2/1"
        }, 3));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "filterDelaysOnly", "Delays Only", true));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "send", "Send", 0.0f, 1.0f, 1.0f));

    return { params.begin(), params.end() };
}