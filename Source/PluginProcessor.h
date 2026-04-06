#pragma once
static constexpr int MAX_DELAY_COUNT = 16;
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class CDelayAudioProcessor : public juce::AudioProcessor
{
public:
    CDelayAudioProcessor();
    ~CDelayAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    std::array<float, MAX_DELAY_COUNT> repeatGains;

    juce::String volumeBarState;

    std::array<float, MAX_DELAY_COUNT> barValues;
    std::array<float, MAX_DELAY_COUNT> panValues;
    std::array<float, MAX_DELAY_COUNT> feedbackValues;
    std::array<float, MAX_DELAY_COUNT> perTapFilterValues;
    std::array<float, MAX_DELAY_COUNT> widthValues;

    float lastMsValue = 500.0f;
    float lastDivisionValue = 4.0f;

private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;
    int delayCount = 4;
    int delayTimeSamples = 22050;
    double currentSampleRate = 44100.0;

    juce::SmoothedValue<float> inputVolumeSmoothed;
    juce::SmoothedValue<float> dryWetSmoothed;
    juce::SmoothedValue<float> outputVolumeSmoothed;

    juce::dsp::IIR::Filter<float> lowPassFilterL;
    juce::dsp::IIR::Filter<float> lowPassFilterR;
    juce::dsp::IIR::Filter<float> highPassFilterL;
    juce::dsp::IIR::Filter<float> highPassFilterR;

    std::array<juce::dsp::IIR::Filter<float>, MAX_DELAY_COUNT> tapLowPassFilterL;
    std::array<juce::dsp::IIR::Filter<float>, MAX_DELAY_COUNT> tapLowPassFilterR;
    std::array<juce::dsp::IIR::Filter<float>, MAX_DELAY_COUNT> tapHighPassFilterL;
    std::array<juce::dsp::IIR::Filter<float>, MAX_DELAY_COUNT> tapHighPassFilterR;

    juce::dsp::IIR::Filter<float> dryLowPassFilterL;
    juce::dsp::IIR::Filter<float> dryLowPassFilterR;
    juce::dsp::IIR::Filter<float> dryHighPassFilterL;
    juce::dsp::IIR::Filter<float> dryHighPassFilterR;

    juce::SmoothedValue<float> filterSmoothed;

    juce::AudioBuffer<float> wetBuffer;

    bool wasPlaying = false;

    juce::SmoothedValue<float> delayTimeSmoothed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CDelayAudioProcessor)
};
