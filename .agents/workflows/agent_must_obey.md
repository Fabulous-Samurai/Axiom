# 🏎️ AXIOM AGENT MUST OBEY: PERFORMANCE & INTEGRITY MANDATES

Bu dosya, Project AXIOM üzerinde çalışan tüm AI Agent'lar için **KESİN VE DEĞİŞTİRİLEMEZ** kuralları içerir. Bu kurallara uymayan her türlü kod değişikliği "Zenith Pillar" ihlali sayılır ve CI/CD tarafından reddedilir.

---

## 🛡️ ZENITH PILLAR 1: ZERO-ALLOCATION (SIFIR TAHSİS)
**Amac:** Bellek parçalanmasını (Fragmentation) önlemek ve deterministic gecikme (latency) sağlamak.

### ❌ YASAKLANANLAR (FORBIDDEN)
Core modüllerde (`src/`, `include/`, `core/`) şunların kullanımı kesinlikle yasaktır:
- `new`, `delete`, `malloc`, `free`
- `std::vector` (Kullanılması gerekiyorsa `AXIOM::FixedVector` kullanın)
- `std::map`, `std::unordered_map` (Kullanılması gerekiyorsa `robin-map` veya `Arena` tabanlı yapılar kullanın)
- `std::string` (Sadece `main`, `tests` veya `config` aşamalarında serbesttir. Core'da `std::string_view` veya `const char*` kullanın)

### ✅ İZİN VERİLENLER (MANDATED)
- `AXIOM::FixedVector`
- `AXIOM::ArenaAllocator` / `HarmonicArena`
- `std::span` (C++20/23)
- `std::array` (Static storage)

---

## 🛡️ ZENITH PILLAR 5: ZERO-EXCEPTION & NO RTTI
**Amac:** Binary boyutunu küçültmek, stack unwinding maliyetinden kaçınmak ve cache locality'yi korumak.

### ❌ YASAKLANANLAR (FORBIDDEN)
- `throw`, `try`, `catch`
- `dynamic_cast` (RTTI kapalıdır, derleme hatası verir)
- `typeid`

### ✅ İZİN VERİLENLER (MANDATED)
- `std::expected` (C++23) veya `std::optional`
- Error codes (enum veya int)
- `static_cast` veya `reinterpret_cast`

---

## 🚀 ZERO-COPY PRINCIPLES (SIFIR KOPYALAMA)
**Amac:** CPU döngülerini veri kopyalamak yerine işlem yapmak için kullanmak.

### ❌ YASAKLANANLAR (BAD PRACTICE)
- Fonksiyonlara büyük nesneleri (struct, vector, string) değer ile (by-value) geçmek.
- Gereksiz veri dönüşümleri (string -> c_str -> string).

### ✅ İZİN VERİLENLER (BEST PRACTICE)
- **Move Semantics:** `std::move()` ve rvalue references (`&&`).
- **Views:** `std::string_view`, `std::span`.
- **In-place:** `emplace_back` (FixedVector için), `placement new` (sadece Arena içinde).
- **Eigen Integration:** Matris verileri için `Eigen::Map` kullanarak belleği kopyalamadan map'lemek.

---

## ⚡ PERFORMANCE TIPS & INLINING
- **Hot-Path:** Saniyede milyonlarca kez çağrılan fonksiyonları `inline` veya `[[msvc::forceinline]]` (MSVC) / `__attribute__((always_inline))` (GCC) ile işaretleyin.
- **Cache Alignment:** Sık kullanılan struct'ları `alignas(64)` ile L1 Cache satırına hizalayın.
- **SIMD:** Vektörize edilebilecek döngülerde derleyiciye ipucu verin veya `Eigen`'in SIMD optimize edilmiş fonksiyonlarını kullanın.

---

## 📜 MUAFİYETLER (EXEMPTIONS)
Şu dizinlerde/dosyalarda yukarıdaki kurallar esnetilebilir (ancak yine de kaçınılmalıdır):
- `tests/` (Test senaryoları)
- `main.cpp` (Başlangıç konfigürasyonu)
- `scripts/` (Python yardımcı araçları)
- `nanobind/` (Python entegrasyon katmanı)

---
**Agent Warning:** "Senna Speed" seviyesinde performans için bu kurallar projenin anayasasıdır. Herhangi bir değişiklik yapmadan önce `verify_zenith_pillars.py` aracını çalıştırarak uyumluluğu kontrol edin.
