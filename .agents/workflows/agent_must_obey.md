cmake_minimum_required(VERSION 3.12)

# FORCE NINJA GENERATOR - MAXIMUM SPEED BUILD SYSTEM

set(CMAKE_GENERATOR "Ninja" CACHE STRING "Generator" FORCE)

# Project and Version

project(axiom_engine VERSION 3.1.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================

# AXIOM ZENITH RECOVERY: PILLAR 1 & PILLAR 5 ENFORCEMENT

# ============================================================================

# Neden (The Why): C++ exceptions (hata fırlatma), "Stack Unwinding" adı verilen ve

# işletim sisteminin bellek yöneticisini tetikleyen devasa bir dinamik tahsis sürecine neden olur

# RTTI (dynamic_cast vs.), çalışma zamanı tip bilgisi ekleyerek nesneleri şişirir ve L1 önbellek

# yerelliğini (Cache Locality) bozar. Bu nedenle derleyici seviyesinde kesin olarak yasaklanmıştır

add_compile_options(-fno-exceptions -fno-rtti)

# Neden (The Why): Modüller Arası Optimizasyon (LTO), sadece belirli bayraklara bağlı kalmamalıdır

# Tüm projenin tek bir devasa derleme ünitesi gibi birleştirilmesi, derleyicinin SPSC kuyrukları

# ve SoA mimarilerindeki fonksiyonları sınır tanımadan inline etmesini (satır içine almasını) sağlar

cmake_policy(SET CMP0069 NEW)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# SIMD / AVX configuration

option(AXIOM_ENABLE_SIMD_AVX2 "Enable AVX2 SIMD instructions" ON)
option(AXIOM_ENABLE_SIMD_FMA "Enable FMA instructions" ON)
option(AXIOM_ENABLE_SIMD_AVX512 "Enable AVX-512 SIMD instructions" OFF)
option(AXIOM_ENABLE_SIMD_AVX_VNNI "Enable AVX-VNNI integer dot-product instructions (Alder Lake+/Zen4+)" OFF)

# ... (Mevcut opsiyonlar korunur)

option(AXIOM_AUTO_INSTALL_PYTHON_DEPS "Install optional Python GUI dependencies via pip" ON)
option(AXIOM_ENABLE_EMBEDDED_PYTHON_ENGINE "Build embedded PythonEngine/PythonParser/PythonREPL sources" OFF)
option(AXIOM_ENABLE_CXX20_MODULES "Enable incremental C++20 modules migration (experimental)" OFF)
option(AXIOM_ENABLE_NANOBIND "Enable nanobind FFI integration" ON)
option(AXIOM_ENABLE_DOXYGEN "Enable Doxygen API documentation target" OFF)
option(AXIOM_ENABLE_HARMONIC_ARENA "Enable HarmonicArena lock-free fast-path backend" OFF)
option(AXIOM_ENABLE_MOLD_LINKER "Try to use mold linker for faster incremental links" OFF)
option(AXIOM_ENABLE_SIZE_GUARDS "Enable section GC and symbol stripping in Release" ON)
option(AXIOM_AUTO_SIGN_WINDOWS "Automatically Authenticode-sign Windows binaries after build" ON)
set(AXIOM_WINDOWS_SIGNING_SUBJECT "CN=AXIOM Local Dev Code Signing" CACHE STRING "Subject for local Windows code-signing certificate")
set(AXIOM_PY_REQUIREMENTS "${CMAKE_SOURCE_DIR}/requirements-optional.txt")

# Detect common CI environments and adjust unsafe, machine-specific defaults

set(AXIOM_RUNNING_IN_CI OFF)
if(DEFINED ENV{GITHUB_ACTIONS} OR DEFINED ENV{CI})
    set(AXIOM_RUNNING_IN_CI ON)
    message(STATUS "CI detected: Adjusting build defaults for reproducibility and portability")
    set(AXIOM_AUTO_INSTALL_PYTHON_DEPS OFF CACHE BOOL "Install optional Python GUI dependencies via pip" FORCE)
    set(AXIOM_AUTO_SIGN_WINDOWS OFF CACHE BOOL "Automatically Authenticode-sign Windows binaries after build" FORCE)
endif()

# Enable position independent code for nanobind

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# NINJA PARALLEL BUILD OPTIMIZATION - ALWAYS ENABLED

include(ProcessorCount)
ProcessorCount(CPU_COUNT)
if(NOT CPU_COUNT EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${CPU_COUNT})
else()
    set(CMAKE_BUILD_PARALLEL_LEVEL 8)  # Fallback
endif()

# NINJA-ONLY OPTIMIZATION FLAGS - SENNA SPEED

# Neden (The Why): LTO artık CMAKE_INTERPROCEDURAL_OPTIMIZATION ile global olarak sağlandığından

# buradaki manuel -flto bayrakları temizlendi. Donanım ön-getiricisini (prefetcher) maksimize

# etmek için native mimari hedefleri korundu

if(AXIOM_RUNNING_IN_CI)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -ffast-math" CACHE STRING "Release flags" FORCE)
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -DNDEBUG -ffast-math" CACHE STRING "Release flags" FORCE)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags" FORCE)

message(STATUS "🏎️ NINJA OPTIMIZATION: Release mode with native CPU optimizations, No-Exceptions, No-RTTI, Global LTO")

# ... (Dosyanın geri kalanı nanobind, Eigen3, test ve SIMD hedefleriyle aynı şekilde devam eder)
