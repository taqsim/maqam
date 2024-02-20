#
# Dependency: SFZ Player <- Sfizz SFZ parser <- Abseil
#
set(ABSEIL_DIR ${THIRDPARTY_DIR}/abseil-cpp)
add_subdirectory(${ABSEIL_DIR} ${CMAKE_CURRENT_BINARY_DIR}/absl)
target_include_directories(${PROJECT_NAME} PRIVATE ${ABSEIL_DIR})

target_link_libraries(
        ${PROJECT_NAME}
        absl::flat_hash_map
        absl::strings
)

#
# Dependency: SFZ Player <- Sfizz SFZ parser
#
set(SFIZZ_PARSER_DIR ${THIRDPARTY_DIR}/sfizz_parser)
target_include_directories(${PROJECT_NAME} PRIVATE ${SFIZZ_PARSER_DIR})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${SFIZZ_PARSER_DIR}/Opcode.cpp
        ${SFIZZ_PARSER_DIR}/parser/Parser.cpp
        ${SFIZZ_PARSER_DIR}/parser/ParserPrivate.cpp
)

#
# Dependency: SFZ Player <- AKSampler <- WavPack
#
set(WAVPACK_DIR ${THIRDPARTY_DIR}/wavpack)
target_include_directories(${PROJECT_NAME} PRIVATE ${WAVPACK_DIR})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${WAVPACK_DIR}/common_utils.c
        ${WAVPACK_DIR}/decorr_utils.c
        ${WAVPACK_DIR}/entropy_utils.c
        ${WAVPACK_DIR}/extra1.c
        ${WAVPACK_DIR}/extra2.c
        ${WAVPACK_DIR}/open_filename.c
        ${WAVPACK_DIR}/open_legacy.c
        ${WAVPACK_DIR}/open_raw.c
        ${WAVPACK_DIR}/open_utils.c
        ${WAVPACK_DIR}/pack.c
        ${WAVPACK_DIR}/pack_dns.c
        ${WAVPACK_DIR}/pack_dsd.c
        ${WAVPACK_DIR}/pack_floats.c
        ${WAVPACK_DIR}/pack_utils.c
        ${WAVPACK_DIR}/read_words.c
        ${WAVPACK_DIR}/tag_utils.c
        ${WAVPACK_DIR}/tags.c
        ${WAVPACK_DIR}/unpack.c
        ${WAVPACK_DIR}/unpack3.c
        ${WAVPACK_DIR}/unpack3_open.c
        ${WAVPACK_DIR}/unpack3_seek.c
        ${WAVPACK_DIR}/unpack_dsd.c
        ${WAVPACK_DIR}/unpack_floats.c
        ${WAVPACK_DIR}/unpack_seek.c
        ${WAVPACK_DIR}/unpack_utils.c
        ${WAVPACK_DIR}/write_words.c
)

#
# Dependency: SFZ Player <- AKSampler + TAQS.IM patches
#
set(AKSAMPLER_DIR ${NODES_DIR}/ak_sampler)

target_include_directories(${PROJECT_NAME} PRIVATE ${AKSAMPLER_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${AKSAMPLER_DIR}/dsp/Common)
target_include_directories(${PROJECT_NAME} PRIVATE ${AKSAMPLER_DIR}/dsp/Plugin)
target_include_directories(${PROJECT_NAME} PRIVATE ${AKSAMPLER_DIR}/dsp/Sampler)

target_link_libraries(
        ${PROJECT_NAME}
        juce::juce_audio_formats
)

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${AKSAMPLER_DIR}/dsp/Common/ADSREnvelope.cpp
        ${AKSAMPLER_DIR}/dsp/Common/EnvelopeGeneratorBase.cpp
        ${AKSAMPLER_DIR}/dsp/Common/FunctionTable.cpp
        ${AKSAMPLER_DIR}/dsp/Common/MultiStageFilter.cpp
        ${AKSAMPLER_DIR}/dsp/Common/ResonantLowPassFilter.cpp
        ${AKSAMPLER_DIR}/dsp/Common/VoiceBase.cpp
        ${AKSAMPLER_DIR}/dsp/Common/VoiceManager.cpp
        ${AKSAMPLER_DIR}/dsp/Plugin/AKSampler.cpp
        ${AKSAMPLER_DIR}/dsp/Plugin/AKSamplerEditor.cpp
        ${AKSAMPLER_DIR}/dsp/Plugin/AKSamplerProcessor.cpp
        ${AKSAMPLER_DIR}/dsp/Plugin/FilterSelector.cpp
        ${AKSAMPLER_DIR}/dsp/Plugin/GuiComponentUtils.cpp
        ${AKSAMPLER_DIR}/dsp/Plugin/PatchParams.cpp
        ${AKSAMPLER_DIR}/dsp/Sampler/SampleBuffer.cpp
        ${AKSAMPLER_DIR}/dsp/Sampler/Sampler.cpp
        ${AKSAMPLER_DIR}/dsp/Sampler/SamplerVoice.cpp
        ${AKSAMPLER_DIR}/AKSamplerProcessorEx.cpp
        ${AKSAMPLER_DIR}/AKSamplerProcessorExJNI.cpp
)

#
# Dependency: SCReverb and Filter <- Soundpipe
#
set(SOUNDPIPE ${NODES_DIR}/shared/soundpipe)

target_include_directories(${PROJECT_NAME} PRIVATE ${SOUNDPIPE})

target_compile_definitions(${PROJECT_NAME} PRIVATE
        NO_LIBSNDFILE
        SNDFILE=int
        SF_INFO=int
)

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${SOUNDPIPE}/base.c
)

#
# Sean Costello Reverb
#
set(SC_REVERB_DIR ${NODES_DIR}/sc_reverb)

target_include_directories(${PROJECT_NAME} PRIVATE ${SC_REVERB_DIR})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${SC_REVERB_DIR}/dsp/revsc.c
        ${SC_REVERB_DIR}/SCReverbProcessor.cpp
)

#
# Filter
#
set(FILTER_DIR ${NODES_DIR}/filter)

target_include_directories(${PROJECT_NAME} PRIVATE ${FILTER_DIR})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${FILTER_DIR}/dsp/moogladder.c
        ${FILTER_DIR}/FilterProcessor.cpp
)

#
# Delay
#
set(DELAY_DIR ${NODES_DIR}/delay)

target_include_directories(${PROJECT_NAME} PRIVATE ${DELAY_DIR})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${DELAY_DIR}/DelayProcessor.cpp
)

#
# Test sine wave generator
#
target_include_directories(${PROJECT_NAME} PRIVATE ${NODES_DIR}/test)
