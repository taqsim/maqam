plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.kotlin.plugin.serialization") version "2.2.0"
}

android {
    namespace = "im.taqs.maqam"
    compileSdk = 36

    defaultConfig {
        minSdk = 29

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17" // avoid missing symbols during linkage caused by abseil
                arguments += "-DANDROID_STL=c++_shared" // required by oboe
                arguments += "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON" // support 16 KB page sizes
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlin {
        jvmToolchain {
            languageVersion.set(JavaLanguageVersion.of(11))
        }
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
    buildFeatures {
        viewBinding = true
        prefab = true
    }
}

dependencies {
    implementation("com.google.oboe:oboe:1.9.3")
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.9.0")
    implementation("androidx.work:work-runtime-ktx:2.10.2")
    implementation("androidx.constraintlayout:constraintlayout:2.2.1")
    implementation("androidx.test.ext:junit-ktx:1.2.1")
}
