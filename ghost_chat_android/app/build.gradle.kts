plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.ghost.app"
    compileSdk = 35

    defaultConfig {
        applicationId = "com.ghost.app"
        minSdk = 26
        targetSdk = 35
        versionCode = 1
        versionName = "2.0.0"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    buildFeatures {
        viewBinding = true
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}

dependencies {
    // AndroidX Core
    implementation(libs.androidx.appcompat)
    implementation(libs.androidx.core.ktx)
    implementation(libs.material)

    // Networking — OkHttp for ESP32
    implementation("com.squareup.okhttp3:okhttp:4.12.0")
    implementation("com.google.code.gson:gson:2.10.1")

    // QR Code Scanning
    implementation("com.journeyapps:zxing-android-embedded:4.3.0")
    implementation("com.google.zxing:core:3.4.1")

    // Kotlin Coroutines
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.7.3")
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:1.8.1")

    // Lifecycle + ViewModel + StateFlow
    implementation("androidx.lifecycle:lifecycle-runtime-ktx:2.8.7")
    implementation("androidx.lifecycle:lifecycle-viewmodel-ktx:2.8.7")
    implementation("androidx.lifecycle:lifecycle-livedata-ktx:2.8.7")
    implementation("androidx.fragment:fragment-ktx:1.8.5")

    // RecyclerView
    implementation("androidx.recyclerview:recyclerview:1.3.2")

    // SwipeRefreshLayout
    implementation("androidx.swiperefreshlayout:swiperefreshlayout:1.1.0")

    // Security — EncryptedSharedPreferences + Keystore support
    implementation("androidx.security:security-crypto:1.1.0-alpha06")

    // WorkManager — lifecycle-safe background sync
    implementation("androidx.work:work-runtime-ktx:2.10.0")

    // Biometric
    implementation("androidx.biometric:biometric:1.1.0")

    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(libs.androidx.junit)
}