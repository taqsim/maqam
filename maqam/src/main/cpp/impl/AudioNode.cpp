//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#include "AudioNode.h"
#include "NativeWrapper.h"
#include "nodes/ValueTreeProvider.h"

using namespace maqam;

constexpr static int kMaxParameterName = 128;

/**
 * FIXME - Assuming all processor parameters are of type float
 *
 * Since dynamic_cast does not work across shared libraries on Android, operations on parameters
 * belonging to processor instances created by different modules rather than libmaqam.so will fail.
 *
 * If third party processors implement parameters which are not of type float, then dynamic_cast
 * should be used and for the moment such implementations should be inserted into libmaqam.so
 *
 * dynamic_cast<juce::AudioParameterFloat*>(float_parameter_pointer_from_another_module) == nullptr
 *
 * ValueTreeProvider functionality is also affected
 *
 * Reference
 * https://stackoverflow.com/questions/56420705/android-ndk-two-shared-libraries-rtti-dynamic-cast-impossible
 * https://docs.juce.com/master/classAudioProcessorParameter.html
 *
 */
static juce::AudioParameterFloat* asAudioParameterFloat(juce::AudioProcessorParameter* p)
{
    return static_cast<juce::AudioParameterFloat*>(p);
    //return dynamic_cast<juce::AudioParameterFloat*>(p);
}

AudioNode::AudioNode()
    : mJVM(nullptr)
    , mEnv(nullptr)
{}

AudioNode::~AudioNode()
{
    AudioNode::getDSP(mEnv, mOwner)->removeListener(this);
    mEnv->DeleteGlobalRef(mOwner);

    NativeWrapper::deleteImpl(mEnv, mOwner, AudioNode::kDSPFieldName);
}

AudioNode* AudioNode::fromJava(JNIEnv *env, jobject thiz) noexcept
{
    return NativeWrapper::getImpl<AudioNode>(env, thiz);
}

juce::AudioProcessor* AudioNode::getDSP(JNIEnv *env, jobject thiz) noexcept
{
    return NativeWrapper::getImpl<juce::AudioProcessor>(env, thiz, AudioNode::kDSPFieldName);
}

void AudioNode::createDSP(JNIEnv *env, jobject thiz) noexcept
{
    NativeWrapper::createImpl(env, thiz, AudioNode::kDSPFieldName);

    mEnv = env;
    mEnv->GetJavaVM(&mJVM);
    mEnvThreadId = std::this_thread::get_id();
    mOwner = mEnv->NewGlobalRef(thiz);

    juce::AudioProcessor* processor = AudioNode::getDSP(mEnv, mOwner);
    const juce::Array<juce::AudioProcessorParameter*> parameters = processor->getParameters();

    for (juce::AudioProcessorParameter* p : parameters) {
        auto* parameterWithID = asAudioParameterFloat(p);

        if (parameterWithID != nullptr) {
            mAudioProcessorParameters[parameterWithID->getParameterID()] = parameterWithID;
        }
    }

    processor->addListener(this);

    auto* proc = dynamic_cast<maqam::ValueTreeProvider*>(processor);

    if (proc != nullptr) {
        proc->getValueTree().addListener(this);
    }
}

float AudioNode::getParameterValue(const juce::String& id) noexcept
{
    auto* param = mAudioProcessorParameters[id];

    if (param != nullptr) {
        return param->get();
    }

    return 0;
}

void AudioNode::setParameterValue(const juce::String& id, float value) noexcept
{
    auto* param = mAudioProcessorParameters[id];

    if (param != nullptr) {
        param->setValueNotifyingHost(param->convertTo0to1(value));
    }
}

void AudioNode::getParameterValueRange(const juce::String& id, float range[]) noexcept
{
    auto* param = mAudioProcessorParameters[id];

    if (param != nullptr) {
        // juce::NormalisableRange<float> param->range
        range[0] = param->range.start;
        range[1] = param->range.end;
    } else {
        range[0] = range[1] = 0;
    }
}

juce::String AudioNode::getParameterName(const juce::String& id) noexcept
{
    auto* param = mAudioProcessorParameters[id];

    if (param != nullptr) {
        return param->getName(kMaxParameterName);
    }

    return "";
}

juce::String AudioNode::getParameterValueAsText(const juce::String& id) noexcept
{
    auto* param = mAudioProcessorParameters[id];

    if (param != nullptr) {
        return param->getCurrentValueAsText();
    }

    return "";
}

juce::String AudioNode::getParameterLabel(const juce::String& id) noexcept
{
    auto* param = mAudioProcessorParameters[id];

    if (param != nullptr) {
        return param->getLabel();
    }

    return "";
}

float AudioNode::getValueTreePropertyFloatValue(const juce::String& id) noexcept
{
    auto* proc = dynamic_cast<maqam::ValueTreeProvider*>(getDSP(mEnv, mOwner));

    if (proc != nullptr) {
        return proc->getValueTree().getProperty(id);
    }

    return 0.f;
}

void AudioNode::setValueTreePropertyFloatValue(const juce::String& id, const float value)
{
    auto* proc = dynamic_cast<maqam::ValueTreeProvider*>(getDSP(mEnv, mOwner));

    if (proc != nullptr) {
        proc->getValueTree().setProperty(id, value, nullptr);
    }
}

juce::String AudioNode::getValueTreePropertyStringValue(const juce::String& id) noexcept
{
    auto* proc = dynamic_cast<maqam::ValueTreeProvider*>(getDSP(mEnv, mOwner));

    if (proc != nullptr) {
        return proc->getValueTree().getProperty(id);
    }

    return "";
}

void AudioNode::setValueTreePropertyStringValue(const juce::String& id, const juce::String& value)
{
    auto* proc = dynamic_cast<maqam::ValueTreeProvider*>(getDSP(mEnv, mOwner));

    if (proc != nullptr) {
        proc->getValueTree().setProperty(id, value, nullptr);
    }
}

// FIXME - make this RT-safe as AudioNode must deal with arbitrary processors not just TAQS.IM's
// IMPORTANT NOTE: This will be called synchronously when a parameter changes, and many audio
// processors will change their parameter during their audio callback. This means that not only has
// your handler code got to be completely thread-safe, but it's also got to be VERY fast, and avoid
// blocking. If you need to handle this event on your message thread, use this callback to trigger
// an AsyncUpdater or ChangeBroadcaster which you can respond to on the message thread
void AudioNode::audioProcessorParameterChanged(juce::AudioProcessor* processor, int parameterIndex,
                                               float newValue)
{
    juce::AudioProcessorParameter* param = processor->getParameters()[parameterIndex];
    jmethodID method = mEnv->GetMethodID(mEnv->GetObjectClass(mOwner), "jniOnParameterChanged",
                                         "(Ljava/lang/String;F)V");

    if (auto* fparam = asAudioParameterFloat(param)) {
        jstring id = mEnv->NewStringUTF(fparam->getParameterID().toUTF8());
        mEnv->CallVoidMethod(mOwner, method, id, fparam->get());
        mEnv->DeleteLocalRef(id);
    }
}

void AudioNode::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
                                         const juce::Identifier& property)
{
    if (property.toString() == "value") {
        return; // some parameter value changed, ignore.
    }

    const bool shouldAttachCurrentThread = std::this_thread::get_id() != mEnvThreadId;
    JNIEnv* env = mEnv;

    if (shouldAttachCurrentThread) {
        JavaVMAttachArgs args;
        args.version = JNI_VERSION_1_6;
        args.name = nullptr;
        args.group = nullptr;
        mJVM->AttachCurrentThread(&env, &args);
    }

    jstring id = env->NewStringUTF(property.toString().toUTF8());
    const juce::var& value = treeWhosePropertyHasChanged.getProperty(property);

    if (value.isBool() || value.isInt() || value.isDouble()) {
        jmethodID method = env->GetMethodID(env->GetObjectClass(mOwner),
                                            "jniOnValueTreePropertyFloatValueChanged",
                                            "(Ljava/lang/String;F)V");
        env->CallVoidMethod(mOwner, method, id, static_cast<float>(value));
    } else {
        jmethodID method = env->GetMethodID(env->GetObjectClass(mOwner),
                                            "jniOnValueTreePropertyStringValueChanged",
                                            "(Ljava/lang/String;Ljava/lang/String;)V");
        jstring sval = env->NewStringUTF(value.toString().toUTF8());
        env->CallVoidMethod(mOwner, method, id, sval);
        env->DeleteLocalRef(sval);
    }

    env->DeleteLocalRef(id);

    if (shouldAttachCurrentThread) {
        mJVM->DetachCurrentThread();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioNode_jniCreateProcessor(JNIEnv *env, jobject thiz)
{
    AudioNode::fromJava(env, thiz)->createDSP(env, thiz);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_im_taqs_maqam_AudioNode_jniGetParameterName(JNIEnv *env, jobject thiz, jstring id)
{
    const char* cId = env->GetStringUTFChars(id, nullptr);
    juce::String name = AudioNode::fromJava(env, thiz)->getParameterName(cId);
    env->ReleaseStringUTFChars(id, cId);

    return env->NewStringUTF(name.toUTF8());
}

extern "C"
JNIEXPORT jstring JNICALL
Java_im_taqs_maqam_AudioNode_jniGetParameterLabel(JNIEnv *env, jobject thiz, jstring id)
{
    const char* cId = env->GetStringUTFChars(id, nullptr);
    juce::String label = AudioNode::fromJava(env, thiz)->getParameterLabel(cId);
    env->ReleaseStringUTFChars(id, cId);

    return env->NewStringUTF(label.toUTF8());
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_im_taqs_maqam_AudioNode_jniGetParameterValue(JNIEnv *env, jobject thiz, jstring id)
{
    const char* cId = env->GetStringUTFChars(id, nullptr);
    float value = AudioNode::fromJava(env, thiz)->getParameterValue(cId);
    env->ReleaseStringUTFChars(id, cId);

    return value;
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioNode_jniSetParameterValue(JNIEnv *env, jobject thiz, jstring id, jfloat value)
{
    const char* cId = env->GetStringUTFChars(id, nullptr);
    AudioNode::fromJava(env, thiz)->setParameterValue(cId, value);
    env->ReleaseStringUTFChars(id, cId);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_im_taqs_maqam_AudioNode_jniGetParameterValueAsText(JNIEnv *env, jobject thiz, jstring id)
{
    const char* cId = env->GetStringUTFChars(id, nullptr);
    juce::String value = AudioNode::fromJava(env, thiz)->getParameterValueAsText(cId);
    env->ReleaseStringUTFChars(id, cId);

    return env->NewStringUTF(value.toUTF8());
}

extern "C"
JNIEXPORT jfloatArray JNICALL
Java_im_taqs_maqam_AudioNode_jniGetParameterValueRange(JNIEnv *env, jobject thiz, jstring id)
{
    jfloat rangeVal[2];
    const char* cId = env->GetStringUTFChars(id, nullptr);
    AudioNode::fromJava(env, thiz)->getParameterValueRange(cId, rangeVal);
    env->ReleaseStringUTFChars(id, cId);
    jfloatArray result = env->NewFloatArray(2);
    env->SetFloatArrayRegion(result, 0, 2, rangeVal);

    return result;
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_im_taqs_maqam_AudioNode_jniGetValueTreePropertyFloatValue(JNIEnv *env, jobject thiz, jstring id)
{
    const char* cId = env->GetStringUTFChars(id, nullptr);
    const float value = AudioNode::fromJava(env, thiz)->getValueTreePropertyFloatValue(cId);
    env->ReleaseStringUTFChars(id, cId);

    return value;
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioNode_jniSetValueTreePropertyFloatValue(JNIEnv *env, jobject thiz, jstring id, jfloat value)
{
    const char* cId = env->GetStringUTFChars(id, nullptr);

    try {
        AudioNode::fromJava(env, thiz)->setValueTreePropertyFloatValue(cId, value);
    } catch (const std::exception& e) {
        env->ThrowNew(env->FindClass("im/taqs/maqam/Library$Exception"), e.what());
    }

    env->ReleaseStringUTFChars(id, cId);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_im_taqs_maqam_AudioNode_jniGetValueTreePropertyStringValue(JNIEnv *env, jobject thiz, jstring id)
{
    const char* cId = env->GetStringUTFChars(id, nullptr);
    juce::String value = AudioNode::fromJava(env, thiz)->getValueTreePropertyStringValue(cId);
    env->ReleaseStringUTFChars(id, cId);

    return env->NewStringUTF(value.toUTF8());
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioNode_jniSetValueTreePropertyStringValue(JNIEnv *env, jobject thiz, jstring id, jstring value)
{
    const char* cId = env->GetStringUTFChars(id, nullptr);
    const char* cValue = env->GetStringUTFChars(value, nullptr);

    try {
        AudioNode::fromJava(env, thiz)->setValueTreePropertyStringValue(cId, cValue);
    } catch (const std::exception& e) {
        env->ThrowNew(env->FindClass("im/taqs/maqam/Library$Exception"), e.what());
    }

    env->ReleaseStringUTFChars(value, cValue);
    env->ReleaseStringUTFChars(id, cId);
}
