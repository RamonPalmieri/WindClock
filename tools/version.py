Import("env")

import subprocess


def _git_tag_or_fallback(project_dir: str) -> str:
    """Return a clean semver-like version for HA.

    Preferred: exact tag on current commit (e.g. v1.3.5 -> 1.3.5).
    Fallback: short SHA ("dev") when not on a tag.

    Note: To ensure Home Assistant shows only X.Y.Z, build from a tagged commit.
    """

    def _run(args):
        return subprocess.check_output(
            args,
            cwd=project_dir,
            stderr=subprocess.DEVNULL,
        ).decode("utf-8").strip()

    try:
        # Exact tag match (no additional -N-g<sha> suffix)
        tag = _run(["git", "describe", "--tags", "--exact-match"])
        if tag:
            return tag[1:] if tag.startswith("v") else tag
    except Exception:
        pass

    # Not on a tag (or git unavailable) -> keep it simple.
    return "dev"


version = _git_tag_or_fallback(env["PROJECT_DIR"])

# Expose as a C string literal macro.
# Usage in C++: `const char* v = WINDCLOCK_VERSION;`
# PlatformIO passes these defines straight to the compiler; to ensure this becomes
# a proper C string literal we need escaped quotes.
env.Append(CPPDEFINES=[("WINDCLOCK_VERSION", '\\"%s\\"' % version)])
