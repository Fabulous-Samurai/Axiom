import pytest

@pytest.fixture(autouse=True)
def mock_axiom_exe(monkeypatch):
    monkeypatch.setattr("gui_helpers.resolve_axiom_exe", lambda: "/tmp/mock_axiom.exe")
