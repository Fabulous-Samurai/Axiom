import os
import shutil

def cleanup():
    obsolete_files = [
        # Bridge/API Obsolete
        "axiom-master/engine/api/kensui_bridge.cpp",
        
        # Dispatcher Obsolete
        "axiom-master/engine/dispatch/selective_dispatcher.h",
        "axiom-master/engine/dispatch/selective_dispatcher.cpp",
        "axiom-master/engine/dispatch/selective_dispatcher_capi.h",
        "axiom-master/engine/dispatch/selective_dispatcher_capi.cpp",
        
        # UI Obsolete
        "axiom-master/axui/runtime/src/zenith_manager.cpp",
        "axiom-master/axui/runtime/include/axui/zenith_manager.h",
        
        # Core Obsolete (Moved to runtime)
        "axiom-master/engine/core/daemon_engine.cpp",
        "axiom-master/engine/core/secure_vault.cpp",
        "axiom-master/engine/core/cognitive_ecu.cpp",
        "axiom-master/engine/core/sandbox_proxy.cpp",
        "axiom-master/engine/core/cognitive_commander.cpp",
        "axiom-master/engine/core/sentry.cpp",
    ]

    print("🚀 AXIOM REPO CLEANUP INITIATED")
    print("===============================")

    count = 0
    for file_path in obsolete_files:
        if os.path.exists(file_path):
            try:
                os.remove(file_path)
                print(f"✅ Deleted: {file_path}")
                count += 1
            except Exception as e:
                print(f"❌ Error deleting {file_path}: {e}")
        else:
            print(f"⚪ Skipped (Not found): {file_path}")

    print("===============================")
    print(f"🧹 Cleanup complete. Total files removed: {count}")

if __name__ == "__main__":
    cleanup()
