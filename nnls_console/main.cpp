#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#include <vamp-hostsdk/Plugin.h>
#include <vamp-hostsdk/PluginLoader.h>
#include <vamp-hostsdk/RealTime.h>

struct ChordEvent {
    double time;
    std::string chord;
};

struct CliOptions {
    std::string vampPath;
    int stepSize = 16384;
    int fftSize = 16384;
    std::string songPath;
};

struct AudioBuffer {
    std::vector<float> samples;
    int sampleRate = 0;
};

static void setEnvVar(const std::string& name, const std::string& value) {
    if (value.empty()) return;
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
}

static bool parseArgs(int argc, char* argv[], CliOptions& opts) {
    const std::string vampPrefix = "--vamp-path=";
    const std::string stepPrefix = "--step-size=";
    const std::string fftPrefix  = "--fft-size=";
    const std::string songPrefix = "--song=";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind(vampPrefix, 0) == 0) {
            opts.vampPath = arg.substr(vampPrefix.size());
        } else if (arg.rfind(stepPrefix, 0) == 0) {
            opts.stepSize = std::stoi(arg.substr(stepPrefix.size()));
        } else if (arg.rfind(fftPrefix, 0) == 0) {
            opts.fftSize = std::stoi(arg.substr(fftPrefix.size()));
        } else if (arg.rfind(songPrefix, 0) == 0) {
            opts.songPath = arg.substr(songPrefix.size());
        }
    }

    return !opts.songPath.empty() && opts.stepSize > 0 && opts.fftSize > 0;
}

static bool loadWavMono(const std::string& path, AudioBuffer& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    char riff[4];
    uint32_t chunkSize = 0;
    char wave[4];
    file.read(riff, 4);
    file.read(reinterpret_cast<char*>(&chunkSize), 4);
    file.read(wave, 4);

    if (std::string(riff, 4) != "RIFF" || std::string(wave, 4) != "WAVE") {
        return false;
    }

    uint16_t audioFormat = 0;
    uint16_t numChannels = 0;
    uint32_t sampleRate = 0;
    uint16_t bitsPerSample = 0;
    bool foundFmt = false;
    bool foundData = false;

    while (file && !(foundFmt && foundData)) {
        char subId[4];
        uint32_t subSize = 0;
        if (!file.read(subId, 4)) break;
        if (!file.read(reinterpret_cast<char*>(&subSize), 4)) break;

        std::string id(subId, 4);
        if (id == "fmt ") {
            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);
            uint32_t byteRate = 0;
            uint16_t blockAlign = 0;
            file.read(reinterpret_cast<char*>(&byteRate), 4);
            file.read(reinterpret_cast<char*>(&blockAlign), 2);
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);
            if (subSize > 16) {
                file.seekg(subSize - 16, std::ios::cur);
            }
            foundFmt = true;
        } else if (id == "data") {
            if (!foundFmt || numChannels == 0 || sampleRate == 0) return false;

            if (audioFormat == 1 && bitsPerSample == 16) {
                size_t totalSamples = subSize / sizeof(int16_t);
                size_t frames = totalSamples / numChannels;
                std::vector<int16_t> raw(totalSamples);
                file.read(reinterpret_cast<char*>(raw.data()), static_cast<std::streamsize>(subSize));
                out.samples.resize(frames);
                for (size_t f = 0; f < frames; ++f) {
                    out.samples[f] = static_cast<float>(raw[f * numChannels]) / 32768.0f;
                }
            } else if (audioFormat == 3 && bitsPerSample == 32) {
                size_t totalSamples = subSize / sizeof(float);
                size_t frames = totalSamples / numChannels;
                std::vector<float> raw(totalSamples);
                file.read(reinterpret_cast<char*>(raw.data()), static_cast<std::streamsize>(subSize));
                out.samples.resize(frames);
                for (size_t f = 0; f < frames; ++f) {
                    out.samples[f] = raw[f * numChannels];
                }
            } else {
                return false;
            }
            out.sampleRate = static_cast<int>(sampleRate);
            foundData = true;
        } else {
            file.seekg(subSize, std::ios::cur);
        }
    }

    return foundFmt && foundData && out.sampleRate > 0 && !out.samples.empty();
}

static bool loadMp3Mono(const std::string& path, AudioBuffer& out) {
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, path.c_str(), nullptr)) {
        return false;
    }

    const unsigned int channels = mp3.channels > 0 ? mp3.channels : 1;
    const int rate = mp3.sampleRate > 0 ? static_cast<int>(mp3.sampleRate) : 44100;

    std::vector<float> interleaved;
    float chunk[4096 * 2];
    drmp3_uint64 framesRead = 0;
    while ((framesRead = drmp3_read_pcm_frames_f32(&mp3, 4096, chunk)) > 0) {
        size_t count = static_cast<size_t>(framesRead) * channels;
        interleaved.insert(interleaved.end(), chunk, chunk + count);
    }

    drmp3_uninit(&mp3);

    if (interleaved.empty()) {
        return false;
    }

    size_t totalFrames = interleaved.size() / channels;
    out.samples.resize(totalFrames);
    for (size_t f = 0; f < totalFrames; ++f) {
        double sum = 0.0;
        for (unsigned int ch = 0; ch < channels; ++ch) {
            sum += interleaved[f * channels + ch];
        }
        out.samples[f] = static_cast<float>(sum / static_cast<double>(channels));
    }
    out.sampleRate = rate;
    return true;
}

static bool loadAudioMono(const std::string& path, AudioBuffer& out) {
    if (loadWavMono(path, out)) {
        return true;
    }
    return loadMp3Mono(path, out);
}

static double estimateBpm(const std::vector<float>& samples, int sampleRate) {
    if (samples.size() < static_cast<size_t>(sampleRate) * 5 || sampleRate <= 0) {
        return 120.0;
    }

    const int hopSize = std::max(1, sampleRate / 100);
    std::vector<float> env;
    env.reserve(samples.size() / static_cast<size_t>(hopSize));

    for (size_t i = 0; i + static_cast<size_t>(hopSize) < samples.size(); i += static_cast<size_t>(hopSize)) {
        float energy = 0.0f;
        for (int j = 0; j < hopSize; ++j) {
            energy += std::fabs(samples[i + static_cast<size_t>(j)]);
        }
        env.push_back(energy);
    }

    const int minLag = static_cast<int>((60.0 / 180.0) * 100.0);
    const int maxLag = static_cast<int>((60.0 / 60.0) * 100.0);
    if (env.size() <= static_cast<size_t>(maxLag)) return 120.0;

    int bestLag = minLag;
    float maxCorr = -1.0f;

    for (int lag = minLag; lag <= maxLag; ++lag) {
        float corr = 0.0f;
        for (size_t i = 0; i + static_cast<size_t>(lag) < env.size(); ++i) {
            corr += env[i] * env[i + static_cast<size_t>(lag)];
        }
        if (corr > maxCorr) {
            maxCorr = corr;
            bestLag = lag;
        }
    }

    double bpm = 60.0 / (static_cast<double>(bestLag) / 100.0);
    bpm = std::max(60.0, std::min(180.0, bpm));
    return std::round(bpm * 100.0) / 100.0;
}

static std::string estimateKeyFromChords(const std::vector<ChordEvent>& chords) {
    if (chords.empty()) return "C Major";
    const char* roots[] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};
    int counts[12] = {0};

    for (const auto& c : chords) {
        if (c.chord.empty() || c.chord == "N" || c.chord == "No chord") continue;
        for (int i = 0; i < 12; ++i) {
            std::string r(roots[i]);
            if (c.chord.rfind(r, 0) == 0) {
                counts[i]++;
                break;
            }
        }
    }

    int bestIdx = 0;
    for (int i = 1; i < 12; ++i) {
        if (counts[i] > counts[bestIdx]) bestIdx = i;
    }
    return std::string(roots[bestIdx]) + " Major";
}

static std::vector<ChordEvent> extractChordsWithNnlsChroma(
    const std::vector<float>& samples,
    int sampleRate,
    int stepSize,
    int fftSize,
    bool& ok)
{
    ok = false;
    std::vector<ChordEvent> result;
    if (samples.empty() || sampleRate <= 0) return result;

    using namespace Vamp;
    using namespace Vamp::HostExt;

    PluginLoader* loader = PluginLoader::getInstance();
    PluginLoader::PluginKey key = loader->composePluginKey("nnls-chroma", "chordino");
    Plugin* plugin = loader->loadPlugin(key, static_cast<float>(sampleRate), PluginLoader::ADAPT_ALL);

    if (!plugin) {
        key = loader->composePluginKey("vamp-chordino", "chordino");
        plugin = loader->loadPlugin(key, static_cast<float>(sampleRate), PluginLoader::ADAPT_ALL);
    }

    if (!plugin) {
        return result;
    }

    int actualStep = stepSize > 0 ? stepSize : 16384;
    int actualBlock = fftSize > 0 ? fftSize : 16384;
    if (actualStep > actualBlock) actualStep = actualBlock / 2;

    if (!plugin->initialise(1, actualStep, actualBlock)) {
        delete plugin;
        return result;
    }

    auto appendFeatures = [&](const Plugin::FeatureSet& features, size_t fallbackFrame) {
        for (const auto& pair : features) {
            for (const auto& feature : pair.second) {
                if (!feature.label.empty()) {
                    double timeSec = feature.hasTimestamp
                        ? (static_cast<double>(feature.timestamp.sec) +
                           static_cast<double>(feature.timestamp.nsec) / 1000000000.0)
                        : (static_cast<double>(fallbackFrame) / static_cast<double>(sampleRate));

                    timeSec = std::round(timeSec * 100.0) / 100.0;
                    if (result.empty() || result.back().chord != feature.label) {
                        result.push_back({timeSec, feature.label});
                    }
                }
            }
        }
    };

    const size_t sampleCount = samples.size();
    size_t currentPos = 0;

    while (currentPos < sampleCount) {
        size_t blockLen = std::min(static_cast<size_t>(actualBlock), sampleCount - currentPos);
        std::vector<float> block(samples.begin() + static_cast<std::ptrdiff_t>(currentPos),
                                 samples.begin() + static_cast<std::ptrdiff_t>(currentPos + blockLen));
        if (blockLen < static_cast<size_t>(actualBlock)) {
            block.resize(static_cast<size_t>(actualBlock), 0.0f);
        }

        RealTime timestamp = RealTime::frame2RealTime(static_cast<int>(currentPos),
                                                      static_cast<unsigned int>(sampleRate));
        const float* audioChannels[1] = { block.data() };
        Plugin::FeatureSet features = plugin->process(audioChannels, timestamp);
        appendFeatures(features, currentPos);

        currentPos += static_cast<size_t>(actualStep);
    }

    Plugin::FeatureSet remaining = plugin->getRemainingFeatures();
    appendFeatures(remaining, sampleCount);

    delete plugin;
    ok = true;
    return result;
}

static std::string escapeJson(const std::string& s) {
    std::ostringstream oss;
    for (char c : s) {
        if (c == '"' || c == '\\') oss << '\\' << c;
        else oss << c;
    }
    return oss.str();
}

int main(int argc, char* argv[]) {
    CliOptions opts;
    if (!parseArgs(argc, argv, opts)) {
        std::cout << "\"success\": false, \"error\": \"Invalid arguments. Usage: "
                     "--vamp-path=/path/to/plugins --step-size=16384 --fft-size=16384 --song=Burebista.mp3\"\n";
        return 1;
    }

    if (!opts.vampPath.empty()) {
        setEnvVar("VAMP_PATH", opts.vampPath);
    }

    AudioBuffer audio;
    if (!loadAudioMono(opts.songPath, audio)) {
        std::cout << "\"success\": false, \"error\": \"Failed to load or decode audio file: "
                  << escapeJson(opts.songPath) << "\"\n";
        return 1;
    }

    bool pluginOk = false;
    std::vector<ChordEvent> chords = extractChordsWithNnlsChroma(
        audio.samples, audio.sampleRate, opts.stepSize, opts.fftSize, pluginOk);

    if (!pluginOk) {
        std::cout << "\"success\": false, \"error\": \"Failed to load or initialize nnls-chroma:chordino\"\n";
        return 1;
    }

    double bpm = estimateBpm(audio.samples, audio.sampleRate);
    std::string key = estimateKeyFromChords(chords);

    std::cout << "\"success\": true, \"bpm\": " << bpm
              << ", \"key\": \"" << escapeJson(key) << "\", \"chords\": [";
    for (size_t i = 0; i < chords.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << "{\"time\": " << chords[i].time
                  << ", \"chord\": \"" << escapeJson(chords[i].chord) << "\"}";
    }
    std::cout << "]\n";

    return 0;
}
