import sys
import os

# scripts/ klasöründeki modülü import edebilmek için yolu ekliyoruz
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../scripts')))

try:
    from sandbox import run_isolated_expression
except ImportError:
    print("sandbox.py bulunamadı. Lütfen '../scripts' klasöründe olduğundan emin olun.")
    sys.exit(1)

def test_safe_expression():
    res = run_isolated_expression("2 + 2")
    assert res == "4", f"Expected 4, got {res}"
    print("[PASS] Safe expression evaluated successfully.")

def test_timeout():
    # Uzun sürecek bir işlem ile zaman aşımı (timeout) kontrolü
    print("[INFO] Testing timeout mechanism...")
    # Fonksiyon çagrisi engellendigi icin devasa bir islem yapiyoruz.
    res = run_isolated_expression("9**9999999")
    
    # Beklentimiz bir hata, timeout veya termination mesajı almaktır.
    is_timeout = "Timeout" in res or "Terminating" in res or "Error" in res or "Exception" in res
    assert is_timeout, f"Expected timeout or termination, got: {res}"
    print("[PASS] Timeout mechanism works as expected.")

def test_file_access():
    # Kum havuzundan dış dünyadaki dosyalara erişim denemesi (os kütüphanesi ile)
    print("[INFO] Testing OS access restriction...")
    res = run_isolated_expression("__import__('os').listdir('.')")
    print(f"File access result: {res}")
    # Beklentimiz attribute access / function calls engellenmesi sonucu hata vermesidir.
    is_blocked = "Error" in res or "Exception" in res or "not allowed" in res
    assert is_blocked, f"Expected access blocked, got: {res}"
    print("[PASS] OS access restriction verified.")

if __name__ == "__main__":
    test_safe_expression()
    test_timeout()
    test_file_access()
    print("\n[SUCCESS] All sandbox security tests completed.")
